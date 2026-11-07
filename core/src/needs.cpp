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

// Where someone is and what they're doing at this time of day. Jobs replace
// most of this in step 4; for now it is a fixed routine by role.
Plan Routine(const Person& p, int minuteOfDay, Season season, float outsideC, float light) {
    const int h = minuteOfDay / 60;
    const int m = minuteOfDay;
    if (h < 6 || h >= 21) return {Place::House, Activity::Sleeping};
    if (p.sick) return {Place::House, Activity::Resting};

    const bool meal = (m >= 6 * 60 + 30 && m < 7 * 60 + 30) || (m >= 11 * 60 + 30 && m < 12 * 60 + 30) ||
                      (m >= 18 * 60 + 30 && m < 19 * 60 + 30);
    if (meal) return {Place::House, Activity::Eating};
    if (m >= 19 * 60 + 30) return {Place::House, Activity::Indoors};

    const bool morning = m < 11 * 60 + 30;
    const bool daylight = light > 0.3f;
    const bool workOutside = daylight && outsideC > -18.0f;
    const bool growing = season != Season::Winter;

    switch (p.role) {
        case Role::Worker:
            if (!workOutside) return {Place::House, Activity::Indoors};
            if (morning || !growing) return {Place::Woodpile, Activity::AtWoodpile};
            return {Place::Garden, Activity::Gardening};
        case Role::Homemaker:
            if (!morning && growing && workOutside && outsideC > 0.0f) {
                return {Place::Garden, Activity::Gardening};
            }
            return {Place::House, Activity::Indoors};
        case Role::Child:
            if (!morning && growing && workOutside && outsideC > 0.0f) {
                return {Place::Garden, Activity::Gardening};
            }
            if (daylight && outsideC > -8.0f) return {Place::Yard, Activity::Playing};
            return {Place::House, Activity::Indoors};
        case Role::Elder:
            if (h == 14 && daylight && outsideC > 8.0f) return {Place::Yard, Activity::Resting};
            return {Place::House, Activity::Resting};
    }
    return {Place::House, Activity::Indoors};
}

}  // namespace

void Simulation::PlanDay(Household& household) {
    const Date date = CurrentDate();
    const float outside = TemperatureC();
    const float light = Light();
    for (Person& p : household.members) {
        if (!p.alive) continue;
        const Plan plan = Routine(p, date.minuteOfDay, date.season, outside, light);
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
        const float recovery = thriving ? needs_.recoveryPerDay * kPerMinute : 0.0f;
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

    // Share whatever there is.
    const float share = std::min(1.0f, h.stores.food / need);
    const bool hadFood = h.stores.food > 0.0f;
    h.stores.food = std::max(0.0f, h.stores.food - need * share);
    h.foodUsedToday += need * share;
    if (hadFood && h.stores.food <= 0.0f) Emit(EventType::FoodRanOut, index);

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

void Simulation::EndOfDay(Household& household) {
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
