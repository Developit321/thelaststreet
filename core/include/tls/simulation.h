#pragma once

#include <cstdint>
#include <vector>

#include "tls/calendar.h"
#include "tls/street.h"
#include "tls/weather.h"

namespace tls {

struct SimConfig {
    CalendarConfig calendar;
    ClimateConfig climate;
};

// What the simulation reports back. The front end (raylib now, Unreal later)
// reacts to these; it never reaches into the simulation's internals.
enum class EventType {
    NewDay,
    NewSeason,
    NewYear,
};

struct Event {
    EventType type = EventType::NewDay;
    int64_t minute = 0;
};

// The whole game state. It only changes through Step() and commands, and
// advances in fixed one-minute steps, so the same inputs always produce the
// same results.
class Simulation {
public:
    Simulation(const SimConfig& config, const StreetLayout& street);

    // Advance the world by one in-game minute.
    void Step();

    // Debug command: jump to the first morning of a season in the current year.
    void JumpToSeason(Season season);

    int64_t Minute() const { return minute_; }
    Date CurrentDate() const { return calendar_.ToDate(minute_); }
    float SeasonProgress() const { return calendar_.SeasonProgress(minute_); }
    float TemperatureC() const { return weather_.TemperatureC(minute_); }
    float SnowCover() const;  // eased between days so it never pops
    float Light() const { return weather_.Light(minute_); }
    bool Snowing() const;

    const Calendar& GetCalendar() const { return calendar_; }
    const Weather& GetWeather() const { return weather_; }
    const StreetLayout& Street() const { return street_; }

    // Returns and clears everything that happened since the last call.
    std::vector<Event> TakeEvents();

private:
    void Emit(EventType type) { events_.push_back({type, minute_}); }

    Calendar calendar_;
    Weather weather_;
    StreetLayout street_;
    int64_t minute_ = 0;
    std::vector<Event> events_;
};

}  // namespace tls
