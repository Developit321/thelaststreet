#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tls {

// A person's part in the household's day. Decides their routine.
enum class Role { Worker, Homemaker, Child, Elder };

// Where on the plot someone is. The front end decides exactly where to draw them.
enum class Place { House, Garden, Woodpile, Yard };

enum class Activity { Sleeping, Eating, Indoors, AtWoodpile, Gardening, Playing, Resting };

enum class DeathCause { None, Cold, Hunger, Sickness };

const char* RoleName(Role role);
const char* ActivityName(Activity activity);
const char* DeathCauseName(DeathCause cause);
bool IsOutside(Place place);

struct Person {
    std::string name;
    int age = 0;
    Role role = Role::Worker;

    bool alive = true;
    float health = 100.0f;  // 0 = dead
    float warmth = 100.0f;  // how warm their body is; 0 = dangerously cold
    float hunger = 0.0f;    // 0 = fed, 100 = starving
    float morale = 70.0f;
    float grief = 0.0f;     // lowers morale after a death in the family, fades over time
    bool sick = false;

    Place place = Place::House;
    Activity activity = Activity::Sleeping;

    DeathCause cause = DeathCause::None;
    int64_t diedAt = -1;  // game minute

    // Running average of today's body warmth, for the morning sickness check.
    float warmthSumToday = 0.0f;
    int warmthSamplesToday = 0;
};

struct Stores {
    float food = 0.0f;      // rations: one adult's food for one day
    float firewood = 0.0f;  // bundles
};

// Every balance number for needs, in one place, loaded from data/needs.json.
struct NeedsConfig {
    // Food
    float adultRationsPerDay = 1.0f;
    float childRationsPerDay = 0.7f;
    float elderRationsPerDay = 0.8f;
    float hungerPerDayUnfed = 35.0f;

    // The house and its stove
    float houseHeatLossPerHour = 0.25f;  // share of the indoor/outdoor gap lost each hour
    float stoveHeatPerHour = 12.0f;      // degrees added per hour at full burn
    float stoveBundlesPerHour = 0.9f;    // firewood burned per hour at full burn
    float cookingBundlesPerMeal = 0.3f;
    float comfortableIndoorC = 18.0f;    // what the family heats to while awake
    float nightIndoorC = 12.0f;          // fire banked low overnight

    // Bodies
    float comfortableC = 16.0f;    // body warmth is full at or above this air temperature
    float freezingC = -6.0f;       // ...and empty at or below this one
    float warmthChangePerHour = 30.0f;
    float coldDamagePerDay = 45.0f;     // health lost per day at zero warmth
    float hungerDamagePerDay = 18.0f;   // health lost per day while starving
    float sicknessDamagePerDay = 9.0f;
    float recoveryPerDay = 8.0f;        // health regained per day when warm, fed and well

    // Sickness
    float sicknessBaseChance = 0.01f;    // per day
    float sicknessColdChance = 0.25f;    // extra per day at zero warmth
    float sicknessRecoverChance = 0.3f;  // per day while warm and fed

    // Vulnerability: multiplies damage and sickness chances
    float childFrailty = 1.3f;
    float elderFrailty = 1.6f;

    // Warnings
    int warnDaysLeft = 10;
};

struct Household {
    std::string id;
    std::string plotId;
    std::string name;  // "The Linds"
    bool isPlayer = false;

    std::vector<Person> members;
    Stores stores;

    float indoorC = 15.0f;
    float stovePower = 0.0f;  // 0..1, how hard the stove is burning

    // Use over the last day, to answer "how long will this last?"
    float foodUsedToday = 0.0f;
    float firewoodUsedToday = 0.0f;
    float firewoodPerDay = 0.0f;  // smoothed

    bool lost = false;  // everyone has died

    int Living() const;
    float FoodNeededPerDay(const NeedsConfig& needs) const;
};

enum class WarningType {
    FirewoodLow,
    FirewoodOut,
    FoodLow,
    FoodOut,
    HouseCold,
    PersonSick,
    PersonWeak,
};

struct Warning {
    WarningType type;
    int person = -1;      // for person warnings
    float value = 0.0f;   // days left, or degrees
    bool urgent = false;
};

}  // namespace tls
