#include "event_log.h"

#include <algorithm>

namespace {
constexpr size_t kKeep = 60;
constexpr int kShow = 7;
}  // namespace

void EventLog::Add(const tls::Simulation& sim, const tls::Event& e) {
    const auto& households = sim.Households();
    const tls::Household* h = e.household >= 0 ? &households[e.household] : nullptr;
    const tls::Person* p = (h && e.person >= 0) ? &h->members[e.person] : nullptr;
    const tls::Date date = sim.GetCalendar().ToDate(e.minute);

    std::string text;
    bool important = false;
    switch (e.type) {
        case tls::EventType::NewDay:
            return;
        case tls::EventType::NewSeason:
            text = std::string(tls::SeasonName(date.season)) + " has come.";
            break;
        case tls::EventType::NewYear:
            text = "A new year begins: " + std::to_string(date.year) + ".";
            break;
        case tls::EventType::PersonFellSick:
            text = p->name + " has fallen sick.";
            important = true;
            break;
        case tls::EventType::PersonRecovered:
            text = p->name + " is feeling better.";
            break;
        case tls::EventType::PersonDied:
            text = p->name + " has died of " + tls::DeathCauseName(p->cause) + ".";
            important = true;
            break;
        case tls::EventType::FoodRanOut:
            text = h->name + " have run out of food.";
            important = true;
            break;
        case tls::EventType::FirewoodRanOut:
            text = h->name + " have run out of firewood.";
            important = true;
            break;
        case tls::EventType::HouseholdLost:
            text = h->name + " are gone.";
            important = true;
            break;
    }
    entries_.push_front({e.minute, text, important});
    while (entries_.size() > kKeep) entries_.pop_back();
}

void EventLog::Draw(const tls::Simulation& sim, Font font, ui::Blockers& blockers) const {
    if (entries_.empty()) return;

    const int shown = std::min<int>(kShow, static_cast<int>(entries_.size()));
    const float lineH = 22.0f;
    const float width = 470.0f;
    const float height = 40.0f + shown * lineH;
    const Rectangle panel{GetScreenWidth() - width - 16.0f, GetScreenHeight() - 36.0f - 12.0f - height,
                          width, height};
    blockers.Add(panel);
    DrawRectangleRounded(panel, 0.05f, 8, ui::kPanel);
    ui::Text(font, "What has happened", {panel.x + 14, panel.y + 10}, 17, ui::kAccent);

    float y = panel.y + 36.0f;
    for (int i = 0; i < shown; ++i) {
        const Entry& e = entries_[i];
        const tls::Date d = sim.GetCalendar().ToDate(e.minute);
        const char* when = TextFormat("%s %d, %02d:%02d", tls::SeasonName(d.season), d.day, d.Hour(), d.Minute());
        ui::Text(font, when, {panel.x + 14, y}, 15, ui::kMuted);
        ui::Text(font, e.text.c_str(), {panel.x + 130, y}, 16,
                 e.important ? Color{236, 170, 140, 255} : ui::kText);
        y += lineH;
    }
}
