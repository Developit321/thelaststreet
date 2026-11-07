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
