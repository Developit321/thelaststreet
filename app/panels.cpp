#include "panels.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

constexpr float kTop = 84.0f;
constexpr float kLeft = 16.0f;
constexpr float kWidth = 340.0f;
constexpr Color kGood{120, 190, 110, 255};
constexpr Color kOkay{226, 186, 96, 255};
constexpr Color kBad{220, 110, 90, 255};
constexpr Color kAmber{200, 140, 50, 255};
constexpr Color kRed{180, 60, 50, 255};

Color LevelColor(float value) {
    if (value >= 60.0f) return kGood;
    if (value >= 30.0f) return kOkay;
    return kBad;
}

void Bar(Rectangle r, float value, Color color) {
    DrawRectangleRounded(r, 0.5f, 4, Fade(BLACK, 0.5f));
    if (value > 0.0f) {
        DrawRectangleRounded({r.x, r.y, r.width * std::clamp(value / 100.0f, 0.0f, 1.0f), r.height}, 0.5f, 4, color);
    }
}

std::string DateText(const tls::Simulation& sim, int64_t minute) {
    const tls::Date d = sim.GetCalendar().ToDate(minute);
    return std::string("Day ") + std::to_string(d.day) + " of " + tls::SeasonName(d.season) + ", " +
           std::to_string(d.year);
}

std::string WarningText(const tls::Simulation& sim, const tls::Household& h, const tls::Warning& w) {
    switch (w.type) {
        case tls::WarningType::FirewoodOut: return "No firewood left: the stove is cold";
        case tls::WarningType::FirewoodLow:
            return TextFormat("Firewood will last %.0f more days at this rate", std::max(1.0f, w.value));
        case tls::WarningType::FoodOut: return "No food left";
        case tls::WarningType::FoodLow:
            return TextFormat("Food will last %.0f more days", std::max(1.0f, w.value));
        case tls::WarningType::HouseCold: return TextFormat("The house is cold: %.0f°C", w.value);
        case tls::WarningType::PersonSick: return h.members[w.person].name + " is sick";
        case tls::WarningType::PersonWeak: return h.members[w.person].name + " is very weak";
        case tls::WarningType::CropReady:
            return TextFormat("The garden is ready: %.0f rations to harvest before the frost", w.value);
        case tls::WarningType::StorageFull:
            return TextFormat("No room for fresh food: %.0f rations rotting. Preserve it", w.value);
    }
    (void)sim;
    return {};
}

std::string GardenText(const tls::Simulation& sim, const tls::Household& h) {
    const tls::Garden& g = h.garden;
    const tls::WorkConfig& work = sim.Work();
    const tls::Season season = sim.CurrentDate().season;
    if (g.planted <= 0.0f) {
        return season == tls::Season::Winter ? "Resting under the snow" : "Not sown yet";
    }
    if (g.growth < 1.0f) {
        if (g.planted < 1.0f && season == tls::Season::Spring) {
            return TextFormat("%.0f%% sown, growing", g.planted * 100.0f);
        }
        return TextFormat("Growing %.0f%%, weeded %.0f%%", g.growth * 100.0f, g.care * 100.0f);
    }
    const float left = h.CropInGround(work);
    if (left >= 1.0f) return TextFormat("Ready: about %.0f rations to bring in", left);
    return TextFormat("Harvested: %.0f rations this year", g.harvested);
}

}  // namespace

float DrawHouseholdPanel(tls::Simulation& sim, Font font, ui::Blockers& blockers, Selection& selection) {
    const int index = sim.PlayerHousehold();
    if (index < 0) return kTop;
    const tls::Household& h = sim.Households()[index];
    const tls::WorkConfig& work = sim.Work();

    const float rowH = 58.0f;
    const float height = 200.0f + h.members.size() * rowH;
    const Rectangle panel{kLeft, kTop, kWidth, height};
    blockers.Add(panel);
    DrawRectangleRounded(panel, 0.04f, 8, ui::kPanel);

    const float x = panel.x + 14.0f;
    const float vx = x + 80.0f;
    float y = panel.y + 12.0f;
    ui::Text(font, h.name.c_str(), {x, y}, 22, ui::kAccent);
    y += 34.0f;

    const float foodDays = sim.FoodDaysLeft(index);
    const float woodDays = sim.FirewoodDaysLeft(index);
    ui::Text(font, "Food", {x, y}, 16, ui::kMuted);
    ui::Text(font, TextFormat("%.0f rations  (about %.0f days)", h.stores.Food(), foodDays), {vx, y}, 16,
             foodDays < 3.0f ? kBad : ui::kText);
    y += 22.0f;
    ui::Text(font, TextFormat("%.0f fresh (room for %.0f), %.0f preserved (room for %.0f)", h.stores.freshFood,
                              work.freshCapacity, h.stores.preservedFood, work.preservedCapacity),
             {vx, y}, 14, h.stores.freshFood > work.freshCapacity ? kBad : ui::kMuted);
    y += 24.0f;
    ui::Text(font, "Firewood", {x, y}, 16, ui::kMuted);
    ui::Text(font, TextFormat("%.0f bundles  (about %.0f days)", h.stores.firewood, woodDays), {vx, y}, 16,
             woodDays < 3.0f ? kBad : ui::kText);
    y += 24.0f;
    ui::Text(font, "House", {x, y}, 16, ui::kMuted);
    const char* stove = h.stores.firewood <= 0.0f ? "stove cold" : (h.stovePower > 0.05f ? "stove lit" : "stove banked");
    ui::Text(font, TextFormat("%.0f°C inside, %s", h.indoorC, stove), {vx, y}, 16,
             h.indoorC < 8.0f ? kBad : ui::kText);
    y += 24.0f;
    ui::Text(font, "Garden", {x, y}, 16, ui::kMuted);
    ui::Text(font, GardenText(sim, h).c_str(), {vx, y}, 16);
    y += 36.0f;

    // The family: click a name to select them; the buttons set their job.
    const float bw = (kWidth - 28.0f - 3 * 4.0f) / 4.0f;
    for (size_t i = 0; i < h.members.size(); ++i) {
        const tls::Person& p = h.members[i];
        const int person = static_cast<int>(i);
        const Rectangle nameRow{panel.x + 6, y - 4, kWidth - 12, 26};
        const bool selected = selection.household == index && selection.person == person;
        const bool hover = CheckCollisionPointRec(GetMousePosition(), nameRow);
        if (selected || hover) DrawRectangleRounded(nameRow, 0.3f, 6, Fade(WHITE, selected ? 0.12f : 0.06f));
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selection.household = index;
            selection.person = person;
        }

        ui::Text(font, p.name.c_str(), {x, y}, 16, p.alive ? ui::kText : ui::kMuted);
        if (!p.alive) {
            ui::Text(font, "died", {x + 240, y}, 15, ui::kMuted);
            y += rowH;
            continue;
        }
        Bar({x + 220, y + 4, 100, 10}, p.health, LevelColor(p.health));
        if (p.sick) ui::Text(font, "sick", {x + 180, y}, 14, kBad);
        y += 26.0f;

        for (int j = 0; j < tls::kJobCount; ++j) {
            const tls::Job job = static_cast<tls::Job>(j);
            const Rectangle r{x + j * (bw + 4.0f), y, bw, 24};
            if (ui::Button(font, r, tls::JobName(job), p.job == job)) sim.SetJob(index, person, job);
        }
        y += rowH - 26.0f;
    }
    return panel.y + panel.height;
}

void DrawPersonPanel(tls::Simulation& sim, Font font, ui::Blockers& blockers, Selection& selection, float top) {
    if (!selection.Any()) return;
    const tls::Household& h = sim.Households()[selection.household];
    const tls::Person& p = h.members[selection.person];

    const Rectangle panel{kLeft, top + 10.0f, kWidth, p.alive ? 245.0f : 110.0f};
    blockers.Add(panel);
    DrawRectangleRounded(panel, 0.05f, 8, ui::kPanel);

    const float x = panel.x + 14.0f;
    float y = panel.y + 12.0f;
    ui::Text(font, p.name.c_str(), {x, y}, 21, ui::kText);
    if (ui::Button(font, {panel.x + kWidth - 34, panel.y + 8, 24, 24}, "x")) selection.Clear();
    y += 28.0f;
    ui::Text(font, TextFormat("%s, age %d", tls::RoleName(p.role), p.age), {x, y}, 15, ui::kMuted);
    y += 26.0f;

    if (!p.alive) {
        const std::string text = "Died on " + DateText(sim, p.diedAt) + ",";
        ui::Text(font, text.c_str(), {x, y}, 16);
        y += 22.0f;
        ui::Text(font, TextFormat("of %s.", tls::DeathCauseName(p.cause)), {x, y}, 16);
        return;
    }

    ui::Text(font, ActivityName(p.activity), {x, y}, 16, ui::kAccent);
    if (p.sick) ui::Text(font, "Sick", {panel.x + kWidth - 60, y}, 16, kBad);
    y += 26.0f;

    // A direct order, or how to give one.
    const bool ordered = p.orderUntil >= 0 && sim.Minute() < p.orderUntil;
    if (ordered) {
        const int64_t left = p.orderUntil - sim.Minute();
        const std::string text = std::string("Sent to ") + tls::PlaceName(p.orderPlace) +
                                 TextFormat(", %dh %02dm left", static_cast<int>(left / 60), static_cast<int>(left % 60));
        ui::Text(font, text.c_str(), {x, y}, 15);
        if (ui::Button(font, {panel.x + kWidth - 90, y - 4, 76, 24}, "Cancel")) {
            sim.CancelOrder(selection.household, selection.person);
        }
    } else {
        ui::Text(font, TextFormat("Job: %s", tls::JobName(p.job)), {x, y}, 15);
        ui::Text(font, "Click a place on your plot to send them there", {x, y + 20}, 13, ui::kMuted);
    }
    y += 50.0f;

    auto stat = [&](const char* label, float value, Color color) {
        ui::Text(font, label, {x, y}, 15, ui::kMuted);
        Bar({x + 80, y + 4, 150, 10}, value, color);
        ui::Text(font, TextFormat("%.0f", value), {x + 240, y}, 15);
        y += 24.0f;
    };
    stat("Health", p.health, LevelColor(p.health));
    stat("Warmth", p.warmth, LevelColor(p.warmth));
    stat("Fed", 100.0f - p.hunger, LevelColor(100.0f - p.hunger));
    stat("Morale", p.morale, LevelColor(p.morale));
}

void DrawWarnings(const tls::Simulation& sim, Font font) {
    const int index = sim.PlayerHousehold();
    if (index < 0) return;
    const tls::Household& h = sim.Households()[index];
    const std::vector<tls::Warning> warnings = sim.Warnings(index);
    if (warnings.empty()) return;

    // Centred row of pills under the top bar, between the side panels.
    std::vector<std::string> texts;
    float total = 0.0f;
    for (size_t i = 0; i < warnings.size() && i < 4; ++i) {
        texts.push_back(WarningText(sim, h, warnings[i]));
        total += MeasureTextEx(font, texts.back().c_str(), 16, 1).x + 28.0f + 8.0f;
    }
    float x = (GetScreenWidth() - total) * 0.5f;
    const float y = 84.0f;
    for (size_t i = 0; i < texts.size(); ++i) {
        const Vector2 dims = MeasureTextEx(font, texts[i].c_str(), 16, 1);
        const Rectangle pill{x, y, dims.x + 28.0f, 30.0f};
        DrawRectangleRounded(pill, 0.5f, 8, warnings[i].urgent ? kRed : kAmber);
        ui::Text(font, texts[i].c_str(), {x + 14.0f, y + 7.0f}, 16);
        x += pill.width + 8.0f;
    }
}

bool DrawGameOver(const tls::Simulation& sim, Font font, ui::Blockers& blockers) {
    const int index = sim.PlayerHousehold();
    const tls::Household& h = sim.Households()[index];

    const float w = static_cast<float>(GetScreenWidth());
    const float hgt = static_cast<float>(GetScreenHeight());
    DrawRectangle(0, 0, static_cast<int>(w), static_cast<int>(hgt), Fade(BLACK, 0.6f));
    blockers.Add({0, 0, w, hgt});

    const float pw = 560.0f;
    const float ph = 210.0f + h.members.size() * 48.0f;
    const Rectangle panel{(w - pw) * 0.5f, (hgt - ph) * 0.5f, pw, ph};
    DrawRectangleRounded(panel, 0.04f, 8, Color{20, 22, 20, 240});

    const float x = panel.x + 30.0f;
    float y = panel.y + 26.0f;
    const std::string title = h.name + " did not survive";
    ui::Text(font, title.c_str(), {x, y}, 30, ui::kAccent);
    y += 46.0f;

    int64_t last = 0;
    for (const tls::Person& p : h.members) last = std::max(last, p.diedAt);
    const int64_t days = (last - sim.GetCalendar().StartMinute()) / tls::kMinutesPerDay + 1;
    const std::string lasted = "They held on for " + std::to_string(days) + " days, until " +
                               DateText(sim, last) + ".";
    ui::Text(font, lasted.c_str(), {x, y}, 18, ui::kMuted);
    y += 40.0f;

    for (const tls::Person& p : h.members) {
        ui::Text(font, TextFormat("%s, %d", p.name.c_str(), p.age), {x, y}, 18);
        const std::string how = "Died on " + DateText(sim, p.diedAt) + ", of " + tls::DeathCauseName(p.cause) + ".";
        ui::Text(font, how.c_str(), {x, y + 22}, 15, ui::kMuted);
        y += 48.0f;
    }

    return ui::Button(font, {panel.x + (pw - 160.0f) * 0.5f, panel.y + ph - 56.0f, 160.0f, 36.0f}, "Start again");
}
