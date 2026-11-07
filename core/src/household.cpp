#include "tls/household.h"

namespace tls {

const char* RoleName(Role role) {
    switch (role) {
        case Role::Worker: return "Worker";
        case Role::Homemaker: return "Homemaker";
        case Role::Child: return "Child";
        case Role::Elder: return "Elder";
    }
    return "?";
}

const char* ActivityName(Activity activity) {
    switch (activity) {
        case Activity::Sleeping: return "Sleeping";
        case Activity::Eating: return "Eating";
        case Activity::Indoors: return "Busy indoors";
        case Activity::ChoppingWood: return "Chopping firewood";
        case Activity::Planting: return "Planting the garden";
        case Activity::Tending: return "Tending the garden";
        case Activity::Harvesting: return "Harvesting";
        case Activity::Preserving: return "Preserving food";
        case Activity::Playing: return "Playing in the yard";
        case Activity::Resting: return "Resting";
        case Activity::Waiting: return "Waiting in the yard";
    }
    return "?";
}

const char* JobName(Job job) {
    switch (job) {
        case Job::Firewood: return "Firewood";
        case Job::Garden: return "Garden";
        case Job::Preserve: return "Preserve";
        case Job::Rest: return "Rest";
    }
    return "?";
}

const char* PlaceName(Place place) {
    switch (place) {
        case Place::House: return "the house";
        case Place::Garden: return "the garden";
        case Place::Woodpile: return "the woodpile";
        case Place::Yard: return "the yard";
    }
    return "?";
}

const char* DeathCauseName(DeathCause cause) {
    switch (cause) {
        case DeathCause::None: return "";
        case DeathCause::Cold: return "the cold";
        case DeathCause::Hunger: return "hunger";
        case DeathCause::Sickness: return "sickness";
    }
    return "?";
}

bool IsOutside(Place place) { return place != Place::House; }

int Household::Living() const {
    int n = 0;
    for (const Person& p : members) n += p.alive ? 1 : 0;
    return n;
}

float Household::FoodNeededPerDay(const NeedsConfig& needs) const {
    float total = 0.0f;
    for (const Person& p : members) {
        if (!p.alive) continue;
        switch (p.role) {
            case Role::Child: total += needs.childRationsPerDay; break;
            case Role::Elder: total += needs.elderRationsPerDay; break;
            default: total += needs.adultRationsPerDay; break;
        }
    }
    return total;
}

float Household::ExpectedHarvest(const WorkConfig& work) const {
    return work.fullHarvest * garden.planted * (0.4f + 0.6f * garden.care);
}

float Household::CropInGround(const WorkConfig& work) const {
    const float left = ExpectedHarvest(work) - garden.harvested - garden.lostToFrost;
    return left > 0.0f ? left : 0.0f;
}

}  // namespace tls
