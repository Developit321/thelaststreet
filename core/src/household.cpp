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
        case Activity::AtWoodpile: return "At the woodpile";
        case Activity::Gardening: return "In the garden";
        case Activity::Playing: return "Playing in the yard";
        case Activity::Resting: return "Resting";
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

}  // namespace tls
