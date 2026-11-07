#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tls {

// A person's part in the household's day. Decides their routine.
enum class Role { Worker, Homemaker, Child, Elder };

// Where on the plot someone is. The front end decides exactly where to draw them.
enum class Place { House, Garden, Woodpile, Yard };

enum class Activity {
    Sleeping,
    Eating,
    Indoors,
    ChoppingWood,
    Planting,
    Tending,
    Harvesting,
    Preserving,
    Playing,
    Resting,
    Waiting,
};

// What the player has asked someone to spend their working hours on.
enum class Job { Firewood, Garden, Preserve, Rest };
constexpr int kJobCount = 4;

enum class DeathCause { None, Cold, Hunger, Sickness };

const char* RoleName(Role role);
const char* ActivityName(Activity activity);
const char* JobName(Job job);
const char* PlaceName(Place place);
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
    Job job = Job::Firewood;

    // A direct order sends someone somewhere right now, until it runs out.
    Place orderPlace = Place::House;
    int64_t orderUntil = -1;  // game minute; -1 = no order

    DeathCause cause = DeathCause::None;
    int64_t diedAt = -1;  // game minute

    // Running average of today's body warmth, for the morning sickness check.
    float warmthSumToday = 0.0f;
    int warmthSamplesToday = 0;
};

// Food is counted in rations: one adult's food for one day.
struct Stores {
    float freshFood = 0.0f;      // spoils within days, faster in warm weather
    float preservedFood = 0.0f;  // dried, salted or smoked: keeps for months
    float firewood = 0.0f;       // bundles

    float Food() const { return freshFood + preservedFood; }
};

// This year's vegetable garden.
struct Garden {
    float planted = 0.0f;    // share of the garden sown, 0..1
    float growth = 0.0f;     // 1 = ready to harvest
    float care = 0.6f;       // weeding and watering, 0..1; sets the size of the harvest
    float harvested = 0.0f;  // rations already brought in this year
    float lostToFrost = 0.0f;
    bool plantingAnnounced = false;
    bool readyAnnounced = false;
    bool harvestAnnounced = false;
};

// Balance numbers for work, the garden and food storage, from data/work.json.
struct WorkConfig {
    // Work speed by role, 1 = a grown worker.
    float workerRate = 1.0f;
    float homemakerRate = 0.8f;
    float childRate = 0.35f;
    float elderRate = 0.3f;
    float sickRate = 0.4f;       // multiplier while sick
    float bitterColdC = -10.0f;  // outdoor work halves below this
    float tooColdC = -22.0f;     // nobody works outdoors below this

    float firewoodPerHour = 1.0f;  // bundles a worker chops in an hour

    // The garden
    float plantingHours = 36.0f;      // worker-hours to sow the whole garden
    float growDays = 24.0f;           // warm days from sowing to harvest
    float growMinTempC = 5.0f;        // days colder than this don't count
    float careDecayPerDay = 0.04f;    // weeds come back
    float carePerHour = 0.04f;        // tending
    float fullHarvest = 300.0f;       // rations from a fully sown, well tended garden
    float harvestPerHour = 8.0f;      // rations a worker brings in per hour
    float frostLossPerDay = 0.5f;     // share of the unharvested crop lost on each frosty day

    // Food storage
    float freshSpoilWarm = 0.04f;   // share lost per day above 12 C
    float freshSpoilMild = 0.015f;  // ...between 2 and 12 C
    float freshSpoilCold = 0.003f;  // ...below 2 C
    float preservedSpoil = 0.0005f;
    float preservePerHour = 3.0f;          // rations a worker preserves per hour
    float preserveLoss = 0.1f;             // share of weight lost when preserving
    float preserveFirewoodPerRation = 0.05f;
    float freshCapacity = 120.0f;
    float preservedCapacity = 250.0f;
    float overflowSpoilPerDay = 0.25f;     // fresh food beyond capacity rots fast

    float orderMinutes = 120.0f;  // how long a direct order lasts
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
    Garden garden;

    float indoorC = 15.0f;
    float stovePower = 0.0f;  // 0..1, how hard the stove is burning

    // Use over the last day, to answer "how long will this last?"
    float foodUsedToday = 0.0f;
    float firewoodUsedToday = 0.0f;
    float firewoodPerDay = 0.0f;  // smoothed

    bool lost = false;  // everyone has died

    int Living() const;
    float FoodNeededPerDay(const NeedsConfig& needs) const;
    // The size of this year's harvest if everything is brought in.
    float ExpectedHarvest(const WorkConfig& work) const;
    float CropInGround(const WorkConfig& work) const;
};

enum class WarningType {
    FirewoodLow,
    FirewoodOut,
    FoodLow,
    FoodOut,
    HouseCold,
    PersonSick,
    PersonWeak,
    CropReady,
    StorageFull,
};

struct Warning {
    WarningType type;
    int person = -1;      // for person warnings
    float value = 0.0f;   // days left, or degrees
    bool urgent = false;
};

}  // namespace tls
