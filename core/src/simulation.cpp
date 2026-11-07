#include "tls/simulation.h"

#include <algorithm>

namespace tls {

Simulation::Simulation(const SimConfig& config, const StreetLayout& street,
                       std::vector<Household> households)
    : calendar_(config.calendar),
      weather_(calendar_, config.climate),
      street_(street),
      needs_(config.needs),
      households_(std::move(households)) {
    minute_ = calendar_.StartMinute();
    for (size_t i = 0; i < households_.size(); ++i) {
        if (households_[i].isPlayer) playerHousehold_ = static_cast<int>(i);
        households_[i].indoorC = std::max(10.0f, TemperatureC());
        PlanDay(households_[i]);
    }
}

void Simulation::Step() {
    const Date before = CurrentDate();
    ++minute_;
    const Date after = CurrentDate();

    if (after.year != before.year) Emit(EventType::NewYear);
    if (after.season != before.season) Emit(EventType::NewSeason);
    const bool newDay = calendar_.DayIndex(minute_) != calendar_.DayIndex(minute_ - 1);
    if (newDay) {
        Emit(EventType::NewDay);
        for (Household& h : households_) EndOfDay(h);
    }

    for (size_t i = 0; i < households_.size(); ++i) TickHousehold(static_cast<int>(i));
}

void Simulation::JumpToSeason(Season season) {
    const Date now = CurrentDate();
    const int64_t morning = int64_t{6} * 60;
    minute_ = calendar_.SeasonStart(now.year, season) + morning;
    Emit(EventType::NewSeason);
    Emit(EventType::NewDay);
    for (Household& h : households_) PlanDay(h);
}

void Simulation::AdjustStores(int household, float food, float firewood) {
    if (household < 0 || household >= static_cast<int>(households_.size())) return;
    Stores& s = households_[household].stores;
    s.food = std::max(0.0f, s.food + food);
    s.firewood = std::max(0.0f, s.firewood + firewood);
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

bool Simulation::PlayerLost() const {
    return playerHousehold_ >= 0 && households_[playerHousehold_].lost;
}

float Simulation::FirewoodDaysLeft(int household) const {
    const Household& h = households_[household];
    // Before a full day has passed, project from how hard the stove is burning now.
    float perDay = h.firewoodPerDay;
    if (perDay <= 0.0f) {
        perDay = h.stovePower * needs_.stoveBundlesPerHour * 24.0f + needs_.cookingBundlesPerMeal * 3.0f;
    }
    perDay = std::max(perDay, 0.1f);
    return h.stores.firewood / perDay;
}

float Simulation::FoodDaysLeft(int household) const {
    const Household& h = households_[household];
    const float perDay = h.FoodNeededPerDay(needs_);
    return perDay > 0.0f ? h.stores.food / perDay : 0.0f;
}

std::vector<Warning> Simulation::Warnings(int household) const {
    std::vector<Warning> out;
    if (household < 0 || household >= static_cast<int>(households_.size())) return out;
    const Household& h = households_[household];
    if (h.lost) return out;

    if (h.stores.firewood <= 0.0f) {
        out.push_back({WarningType::FirewoodOut, -1, 0.0f, true});
    } else if (const float days = FirewoodDaysLeft(household); days < needs_.warnDaysLeft) {
        out.push_back({WarningType::FirewoodLow, -1, days, days < 3.0f});
    }
    if (h.stores.food <= 0.0f) {
        out.push_back({WarningType::FoodOut, -1, 0.0f, true});
    } else if (const float days = FoodDaysLeft(household); days < needs_.warnDaysLeft) {
        out.push_back({WarningType::FoodLow, -1, days, days < 3.0f});
    }
    if (h.indoorC < 8.0f) out.push_back({WarningType::HouseCold, -1, h.indoorC, h.indoorC < 2.0f});

    for (size_t i = 0; i < h.members.size(); ++i) {
        const Person& p = h.members[i];
        if (!p.alive) continue;
        if (p.health < 35.0f) {
            out.push_back({WarningType::PersonWeak, static_cast<int>(i), p.health, true});
        } else if (p.sick) {
            out.push_back({WarningType::PersonSick, static_cast<int>(i), p.health, false});
        }
    }
    std::stable_sort(out.begin(), out.end(),
                     [](const Warning& a, const Warning& b) { return a.urgent && !b.urgent; });
    return out;
}

std::vector<Event> Simulation::TakeEvents() {
    std::vector<Event> out;
    out.swap(events_);
    return out;
}

}  // namespace tls
