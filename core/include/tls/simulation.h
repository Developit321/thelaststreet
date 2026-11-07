#pragma once

#include <cstdint>
#include <vector>

#include "tls/calendar.h"
#include "tls/household.h"
#include "tls/street.h"
#include "tls/weather.h"

namespace tls {

struct SimConfig {
    CalendarConfig calendar;
    ClimateConfig climate;
    NeedsConfig needs;
};

// What the simulation reports back. The front end (raylib now, Unreal later)
// reacts to these; it never reaches into the simulation's internals.
enum class EventType {
    NewDay,
    NewSeason,
    NewYear,
    PersonFellSick,
    PersonRecovered,
    PersonDied,
    FoodRanOut,
    FirewoodRanOut,
    HouseholdLost,
};

struct Event {
    EventType type = EventType::NewDay;
    int64_t minute = 0;
    int household = -1;
    int person = -1;
};

// The whole game state. It only changes through Step() and commands, and
// advances in fixed one-minute steps, so the same inputs always produce the
// same results.
class Simulation {
public:
    Simulation(const SimConfig& config, const StreetLayout& street,
               std::vector<Household> households);

    // Advance the world by one in-game minute.
    void Step();

    // --- Debug commands ---------------------------------------------------
    // Jump to the first morning of a season in the current year.
    void JumpToSeason(Season season);
    // Add (or with negative amounts, remove) stored food and firewood.
    void AdjustStores(int household, float food, float firewood);

    // --- Time and weather -------------------------------------------------
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
    const NeedsConfig& Needs() const { return needs_; }

    // --- Households -------------------------------------------------------
    const std::vector<Household>& Households() const { return households_; }
    int PlayerHousehold() const { return playerHousehold_; }
    bool PlayerLost() const;

    // Problems the household should know about right now, most urgent first.
    std::vector<Warning> Warnings(int household) const;
    float FirewoodDaysLeft(int household) const;
    float FoodDaysLeft(int household) const;

    // Returns and clears everything that happened since the last call.
    std::vector<Event> TakeEvents();

private:
    void Emit(EventType type, int household = -1, int person = -1) {
        events_.push_back({type, minute_, household, person});
    }

    // Per-minute household update, in needs.cpp.
    void TickHousehold(int index);
    void PlanDay(Household& household);
    void Meal(int index);
    void MorningChecks(int index);
    void EndOfDay(Household& household);
    void Kill(int household, int person, DeathCause cause);

    Calendar calendar_;
    Weather weather_;
    StreetLayout street_;
    NeedsConfig needs_;
    std::vector<Household> households_;
    int playerHousehold_ = -1;
    int64_t minute_ = 0;
    std::vector<Event> events_;
};

}  // namespace tls
