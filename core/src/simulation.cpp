#include "tls/simulation.h"

namespace tls {

Simulation::Simulation(const SimConfig& config, const StreetLayout& street)
    : calendar_(config.calendar), weather_(calendar_, config.climate), street_(street) {
    minute_ = calendar_.StartMinute();
}

void Simulation::Step() {
    const Date before = CurrentDate();
    ++minute_;
    const Date after = CurrentDate();

    if (after.year != before.year) Emit(EventType::NewYear);
    if (after.season != before.season) Emit(EventType::NewSeason);
    if (calendar_.DayIndex(minute_) != calendar_.DayIndex(minute_ - 1)) Emit(EventType::NewDay);
}

void Simulation::JumpToSeason(Season season) {
    const Date now = CurrentDate();
    const int64_t morning = int64_t{6} * 60;
    minute_ = calendar_.SeasonStart(now.year, season) + morning;
    Emit(EventType::NewSeason);
    Emit(EventType::NewDay);
}

float Simulation::SnowCover() const {
    // Blend today's cover into tomorrow's across the day.
    const int64_t day = calendar_.DayIndex(minute_);
    const float t = static_cast<float>(minute_ % kMinutesPerDay) / kMinutesPerDay;
    const float today = weather_.SnowCover(day);
    const float tomorrow = weather_.SnowCover(day + 1);
    return today + (tomorrow - today) * t;
}

bool Simulation::Snowing() const {
    return weather_.Snowfall(calendar_.DayIndex(minute_));
}

std::vector<Event> Simulation::TakeEvents() {
    std::vector<Event> out;
    out.swap(events_);
    return out;
}

}  // namespace tls
