#pragma once

#include <string>
#include <vector>

#include "tls/simulation.h"
#include "tls/street.h"

// Reads the files in data/ into core types. This is the only code that
// knows about JSON; in Unreal it is replaced by Unreal's own JSON reader.

struct StreetLoadResult {
    bool ok = false;
    std::string error;
    tls::StreetLayout street;
};

StreetLoadResult LoadStreet(const std::string& path);

// How fast time runs on screen. This belongs to the front end, not the
// simulation: the simulation only ever moves one minute at a time.
struct Pace {
    float realSecondsPerDay = 60.0f;
    std::vector<float> speeds;  // multipliers offered by the speed buttons
};

struct CalendarLoadResult {
    bool ok = false;
    std::string error;
    tls::CalendarConfig calendar;
    tls::ClimateConfig climate;
    Pace pace;
};

CalendarLoadResult LoadCalendar(const std::string& path);

struct NeedsLoadResult {
    bool ok = false;
    std::string error;
    tls::NeedsConfig needs;
};

NeedsLoadResult LoadNeeds(const std::string& path);

struct WorkLoadResult {
    bool ok = false;
    std::string error;
    tls::WorkConfig work;
};

WorkLoadResult LoadWork(const std::string& path);

struct HouseholdsLoadResult {
    bool ok = false;
    std::string error;
    std::vector<tls::Household> households;
};

// Checks each household against the street, so every one lives on a real plot.
HouseholdsLoadResult LoadHouseholds(const std::string& path, const tls::StreetLayout& street);
