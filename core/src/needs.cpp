// Household needs: the stove, meals, body warmth, health, sickness, death and
// daily routines. Part of Simulation, split out to keep simulation.cpp short.

#include <algorithm>

#include "tls/simulation.h"

namespace tls {

namespace {

constexpr float kPerMinute = 1.0f / kMinutesPerDay;  // converts per-day rates

uint32_t Hash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

// A repeatable 0..1 roll for one person on one day.
float Roll(int64_t day, int household, int person, uint32_t channel) {
    uint32_t h = Hash(static_cast<uint32_t>(day) * 2654435761U);
    h = Hash(h ^ (static_cast<uint32_t>(household) * 0x9e3779b9U));
    h = Hash(h ^ (static_cast<uint32_t>(person) * 0x85ebca6bU) ^ channel);
    return static_cast<float>(h) / 4294967295.0f;
}

float Frailty(const Person& p, const NeedsConfig& needs) {
    switch (p.role) {
        case Role::Child: return needs.childFrailty;
        case Role::Elder: return needs.elderFrailty;
        default: return 1.0f;
    }
}

float DailyRation(const Person& p, const NeedsConfig& needs) {
    switch (p.role) {
        case Role::Child: return needs.childRationsPerDay;
        case Role::Elder: return needs.elderRationsPerDay;
        default: return needs.adultRationsPerDay;
    }
}

struct Plan {
    Place place;
    Activity activity;
};

bool IsMealTime(int m) {
    return (m >= 6 * 60 + 30 && m < 7 * 60 + 30) || (m >= 11 * 60 + 30 && m < 12 * 60 + 30) ||
           (m >= 18 * 60 + 30 && m < 19 * 60 + 30);
}

}  // namespace

// Where everyone is and what they're doing right now: a direct order first,
// then sleep and meals, then their job during working hours.
void Simulation::PlanDay(Household& household) {
    const int m = static_cast<int>(minute_ % kMinutesPerDay);
    const int hour = m / 60;
    const float outside = TemperatureC();
    const bool daylight = Light() > 0.3f;
    const bool canWorkOutside = daylight && outside > work_.tooColdC;
    const Activity gardenWork = GardenActivity(household);

    for (Person& p : household.members) {
        if (!p.alive) continue;
        Plan plan{Place::House, Activity::Indoors};

        if (p.orderUntil >= 0 && minute_ < p.orderUntil) {
            // Orders override the routine, even at night or in the cold.
            switch (p.orderPlace) {
                case Place::Woodpile: plan = {Place::Woodpile, Activity::ChoppingWood}; break;
                case Place::Garden:
                    plan = {Place::Garden, gardenWork == Activity::Indoors ? Activity::Waiting : gardenWork};
                    break;
                case Place::House: plan = {Place::House, Activity::Resting}; break;
                case Place::Yard: plan = {Place::Yard, Activity::Waiting}; break;
            }
        } else if (hour < 6 || hour >= 21) {
            plan = {Place::House, Activity::Sleeping};
        } else if (p.sick) {
            plan = {Place::House, Activity::Resting};
        } else if (IsMealTime(m)) {
            plan = {Place::House, Activity::Eating};
        } else if (m >= 19 * 60 + 30 || m < 7 * 60 + 30) {
            plan = {Place::House, Activity::Indoors};
        } else {
            switch (p.job) {
                case Job::Rest:
                    plan = {Place::House, Activity::Resting};
                    break;
                case Job::Firewood:
                    if (canWorkOutside) plan = {Place::Woodpile, Activity::ChoppingWood};
                    break;
                case Job::Garden:
                    if (canWorkOutside && outside > 0.0f && gardenWork != Activity::Indoors) {
                        plan = {Place::Garden, gardenWork};
                    }
                    break;
                case Job::Preserve:
                    if (household.stores.freshFood > 0.01f &&
                        household.stores.preservedFood < work_.preservedCapacity) {
                        plan = {Place::House, Activity::Preserving};
                    }
                    break;
            }
            // Children play outside when there's nothing for them to do.
            if (plan.activity == Activity::Indoors && p.role == Role::Child && daylight && outside > -8.0f) {
                plan = {Place::Yard, Activity::Playing};
            }
        }
        p.place = plan.place;
        p.activity = plan.activity;
    }
}

void Simulation::TickHousehold(int index) {
    Household& h = households_[index];
    if (h.lost) return;

    const int minuteOfDay = static_cast<int>(minute_ % kMinutesPerDay);
    const float outside = TemperatureC();

    PlanDay(h);
    if (minuteOfDay == 6 * 60) MorningChecks(index);
    if (minuteOfDay == 7 * 60 || minuteOfDay == 12 * 60 || minuteOfDay == 19 * 60) Meal(index);
    DoWork(index);

    // The stove: the family keeps the house warm while awake and banks the
    // fire low overnight, as long as there is wood.
    const bool awake = minuteOfDay >= 6 * 60 && minuteOfDay < 21 * 60;
    const float target = awake ? needs_.comfortableIndoorC : needs_.nightIndoorC;
    float power = std::clamp((target - h.indoorC) / 2.0f, 0.0f, 1.0f);
    float burn = power * needs_.stoveBundlesPerHour / 60.0f;
    if (burn > h.stores.firewood) {
        power = burn > 0.0f ? power * h.stores.firewood / burn : 0.0f;
        burn = h.stores.firewood;
    }
    const bool hadWood = h.stores.firewood > 0.0f;
    h.stores.firewood -= burn;
    h.firewoodUsedToday += burn;
    if (hadWood && h.stores.firewood <= 0.0f) {
        h.stores.firewood = 0.0f;
        Emit(EventType::FirewoodRanOut, index);
    }
    h.stovePower = power;
    h.indoorC += (outside - h.indoorC) * needs_.houseHeatLossPerHour / 60.0f +
                 power * needs_.stoveHeatPerHour / 60.0f;

    // Bodies.
    for (size_t i = 0; i < h.members.size(); ++i) {
        Person& p = h.members[i];
        if (!p.alive) continue;

        // Working outside keeps you a little warmer than standing still.
        const float air = IsOutside(p.place) ? outside + 3.0f : h.indoorC;
        const float span = needs_.comfortableC - needs_.freezingC;
        const float warmthTarget = std::clamp((air - needs_.freezingC) / span, 0.0f, 1.0f) * 100.0f;
        const float warmthStep = needs_.warmthChangePerHour / 60.0f;
        if (p.warmth < warmthTarget) p.warmth = std::min(warmthTarget, p.warmth + warmthStep);
        else p.warmth = std::max(warmthTarget, p.warmth - warmthStep);
        p.warmthSumToday += p.warmth;
        ++p.warmthSamplesToday;

        p.hunger = std::min(100.0f, p.hunger + needs_.hungerPerDayUnfed * kPerMinute);

        const float frailty = Frailty(p, needs_);
        const float cold = p.warmth < 40.0f ? needs_.coldDamagePerDay * (40.0f - p.warmth) / 40.0f : 0.0f;
        const float starving = p.hunger > 60.0f ? needs_.hungerDamagePerDay * (p.hunger - 60.0f) / 40.0f : 0.0f;
        const float sickness = p.sick ? needs_.sicknessDamagePerDay : 0.0f;
        const float damage = (cold + starving + sickness) * frailty * kPerMinute;
        const bool thriving = !p.sick && p.warmth > 60.0f && p.hunger < 40.0f;
        // Resting at home heals twice as fast.
        const float restBonus = p.activity == Activity::Resting && p.place == Place::House ? 2.0f : 1.0f;
        const float recovery = thriving ? needs_.recoveryPerDay * restBonus * kPerMinute : 0.0f;
        p.health = std::clamp(p.health - damage + recovery, 0.0f, 100.0f);

        p.grief = std::max(0.0f, p.grief - 3.0f * kPerMinute);
        const float moraleTarget = std::clamp(
            75.0f - p.hunger * 0.3f - (100.0f - p.warmth) * 0.25f - p.grief - (p.sick ? 15.0f : 0.0f),
            0.0f, 100.0f);
        const float moraleStep = 20.0f * kPerMinute;
        if (p.morale < moraleTarget) p.morale = std::min(moraleTarget, p.morale + moraleStep);
        else p.morale = std::max(moraleTarget, p.morale - moraleStep);

        if (p.health <= 0.0f) {
            DeathCause cause = DeathCause::Cold;
            if (starving >= cold && starving >= sickness) cause = DeathCause::Hunger;
            else if (sickness > cold) cause = DeathCause::Sickness;
            Kill(index, static_cast<int>(i), cause);
            if (h.lost) return;
        }
    }
}

void Simulation::Meal(int index) {
    Household& h = households_[index];

    float need = 0.0f;
    for (const Person& p : h.members) {
        if (p.alive) need += DailyRation(p, needs_) / 3.0f;
    }
    if (need <= 0.0f) return;

    // Share whatever there is, eating fresh food first because it spoils.
    Stores& s = h.stores;
    const bool hadFood = s.Food() > 0.0f;
    const float share = std::min(1.0f, s.Food() / need);
    const float eaten = need * share;
    const float fromFresh = std::min(s.freshFood, eaten);
    s.freshFood -= fromFresh;
    s.preservedFood = std::max(0.0f, s.preservedFood - (eaten - fromFresh));
    h.foodUsedToday += eaten;
    if (hadFood && s.Food() <= 0.0001f) {
        s.freshFood = s.preservedFood = 0.0f;
        Emit(EventType::FoodRanOut, index);
    }

    for (Person& p : h.members) {
        if (!p.alive) continue;
        // A full meal clears a little more than a third of a day's hunger,
        // so fed people stay comfortably fed.
        p.hunger = std::max(0.0f, p.hunger - needs_.hungerPerDayUnfed / 3.0f * 1.25f * share);
    }

    // Cooking takes a little wood, if there is any. Without it, they eat cold.
    const float cooking = std::min(h.stores.firewood, needs_.cookingBundlesPerMeal);
    if (cooking > 0.0f) {
        h.stores.firewood -= cooking;
        h.firewoodUsedToday += cooking;
        if (h.stores.firewood <= 0.0f) {
            h.stores.firewood = 0.0f;
            Emit(EventType::FirewoodRanOut, index);
        }
    }
}

void Simulation::MorningChecks(int index) {
    Household& h = households_[index];
    const int64_t day = calendar_.DayIndex(minute_);

    for (size_t i = 0; i < h.members.size(); ++i) {
        Person& p = h.members[i];
        if (!p.alive) continue;
        const float avgWarmth =
            p.warmthSamplesToday > 0 ? p.warmthSumToday / p.warmthSamplesToday : p.warmth;
        p.warmthSumToday = 0.0f;
        p.warmthSamplesToday = 0;

        const int person = static_cast<int>(i);
        if (!p.sick) {
            // A cold night makes sickness much more likely.
            const float chance = (needs_.sicknessBaseChance +
                                  needs_.sicknessColdChance * (1.0f - avgWarmth / 100.0f)) *
                                 Frailty(p, needs_);
            if (Roll(day, index, person, 1) < chance) {
                p.sick = true;
                Emit(EventType::PersonFellSick, index, person);
            }
        } else if (p.warmth > 60.0f && p.hunger < 40.0f &&
                   Roll(day, index, person, 2) < needs_.sicknessRecoverChance) {
            p.sick = false;
            Emit(EventType::PersonRecovered, index, person);
        }
    }
}

void Simulation::EndOfDay(int index) {
    Household& household = households_[index];
    GardenAndStorageDaily(index, calendar_.DayIndex(minute_) - 1);

    const float used = household.firewoodUsedToday;
    household.firewoodPerDay =
        household.firewoodPerDay <= 0.0f ? used : household.firewoodPerDay * 0.5f + used * 0.5f;
    household.firewoodUsedToday = 0.0f;
    household.foodUsedToday = 0.0f;
}

void Simulation::Kill(int household, int person, DeathCause cause) {
    Household& h = households_[household];
    Person& p = h.members[person];
    p.alive = false;
    p.health = 0.0f;
    p.sick = false;
    p.cause = cause;
    p.diedAt = minute_;
    Emit(EventType::PersonDied, household, person);

    for (Person& other : h.members) {
        if (other.alive) other.grief = std::min(60.0f, other.grief + 40.0f);
    }
    if (h.Living() == 0) {
        h.lost = true;
        Emit(EventType::HouseholdLost, household);
    }
}

}  // namespace tls
