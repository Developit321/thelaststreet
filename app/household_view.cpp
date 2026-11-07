#include "household_view.h"

#include <algorithm>
#include <cmath>

#include "raymath.h"

namespace {

constexpr float kPersonRadius = 0.7f;
constexpr float kWalkSpeed = 18.0f;  // metres per real second on screen
constexpr Color kLog{136, 96, 58, 255};
constexpr Color kLogEnd{196, 160, 112, 255};
constexpr Color kWindowLight{255, 206, 120, 255};
constexpr Color kSelected{226, 186, 96, 255};

Color RoleColor(tls::Role role) {
    switch (role) {
        case tls::Role::Worker: return {70, 96, 140, 255};
        case tls::Role::Homemaker: return {150, 70, 70, 255};
        case tls::Role::Child: return {214, 170, 70, 255};
        case tls::Role::Elder: return {130, 120, 150, 255};
    }
    return GRAY;
}

}  // namespace

const tls::Plot* HouseholdView::PlotOf(const tls::Simulation& sim, int household) const {
    return sim.Street().FindPlot(sim.Households()[household].plotId);
}

Vector2 HouseholdView::TargetPosition(const tls::Simulation& sim, int household, int person) const {
    const tls::Plot* plot = PlotOf(sim, household);
    if (!plot) return {0, 0};
    const tls::Person& p = sim.Households()[household].members[person];
    const float spread = static_cast<float>(person) * 1.6f;
    const bool north = plot->side == tls::StreetSide::North;

    switch (p.place) {
        case tls::Place::House: {
            const tls::Vec2 c = plot->house.Center();
            return {c.x - 2.4f + spread, c.y + (person % 2 == 0 ? -1.0f : 1.0f)};
        }
        case tls::Place::Garden:
            return {plot->garden.x + 2.0f + spread * 1.5f, plot->garden.Center().y};
        case tls::Place::Woodpile:
            return {plot->woodpile.x - 1.2f, plot->woodpile.Center().y + spread * 0.6f};
        case tls::Place::Yard: {
            const tls::Vec2 a = plot->house.Center();
            const tls::Vec2 b = plot->woodpile.Center();
            const float y = north ? a.y - 8.0f : a.y + 8.0f;
            return {(a.x + b.x) * 0.5f + spread, y};
        }
    }
    return {0, 0};
}

void HouseholdView::Update(const tls::Simulation& sim, float dt) {
    const auto& households = sim.Households();
    for (size_t h = 0; h < households.size(); ++h) {
        for (size_t i = 0; i < households[h].members.size(); ++i) {
            const auto key = std::make_pair(static_cast<int>(h), static_cast<int>(i));
            const Vector2 target = TargetPosition(sim, key.first, key.second);
            auto it = drawn_.find(key);
            if (it == drawn_.end()) {
                drawn_[key] = target;
                continue;
            }
            it->second = Vector2MoveTowards(it->second, target, kWalkSpeed * dt);
        }
    }
}

void HouseholdView::DrawWorld(const tls::Simulation& sim, const SceneLook& look,
                              int selectedHousehold, int selectedPerson) const {
    const auto& households = sim.Households();
    const float time = static_cast<float>(GetTime());

    for (size_t h = 0; h < households.size(); ++h) {
        const tls::Household& household = households[h];
        const tls::Plot* plot = PlotOf(sim, static_cast<int>(h));
        if (!plot) continue;

        // Woodpile: rows of logs that shrink as the store runs down.
        const tls::Rect& w = plot->woodpile;
        const float fill = std::clamp(household.stores.firewood / 150.0f, 0.0f, 1.0f);
        const int rows = static_cast<int>(std::ceil(fill * 4.0f));
        for (int r = 0; r < rows; ++r) {
            const float y = w.Bottom() - (r + 1) * (w.h / 4.0f);
            const float rowFill = std::clamp(fill * 4.0f - r, 0.0f, 1.0f);
            const float width = w.w * rowFill;
            DrawRectangleRec({w.x, y, width, w.h / 4.0f - 0.1f}, kLog);
            for (float lx = w.x + 0.35f; lx < w.x + width; lx += 0.7f) {
                DrawCircleV({lx, y + w.h / 8.0f}, 0.28f, kLogEnd);
            }
        }
        if (household.stores.firewood <= 0.0f) {
            DrawRectangleLinesEx({w.x, w.y, w.w, w.h}, 0.15f, Fade(kLog, 0.6f));
        }

        // Lit windows at night while the family is home and has light.
        if (!household.lost && look.darkness > 0.2f) {
            const tls::Rect& hr = plot->house;
            const float glow = look.darkness * (household.stovePower > 0.05f ? 1.0f : 0.5f);
            const float wy = plot->side == tls::StreetSide::North ? hr.Bottom() - 0.5f : hr.y;
            for (int i = 0; i < 3; ++i) {
                DrawRectangleRec({hr.x + 1.5f + i * 3.0f, wy - 0.25f, 1.4f, 0.5f}, Fade(kWindowLight, glow));
            }
        }

        // Chimney smoke while the stove burns.
        if (household.stovePower > 0.05f) {
            const tls::Rect& hr = plot->house;
            const Vector2 chimney{hr.x + hr.w * 0.72f + 0.55f, hr.y + hr.h * 0.22f + 0.55f};
            for (int i = 0; i < 6; ++i) {
                const float age = std::fmod(time * 0.6f + i / 6.0f, 1.0f);
                const Vector2 puff{chimney.x + age * 4.0f + std::sin(time + i) * 0.4f,
                                   chimney.y - age * 3.0f};
                DrawCircleV(puff, 0.5f + age * 1.4f,
                            Fade(Color{200, 200, 200, 255}, (1.0f - age) * 0.35f * household.stovePower));
            }
        }

        // People.
        for (size_t i = 0; i < household.members.size(); ++i) {
            const tls::Person& p = household.members[i];
            if (!p.alive) continue;
            const auto it = drawn_.find({static_cast<int>(h), static_cast<int>(i)});
            if (it == drawn_.end()) continue;
            const Vector2 pos = it->second;
            const bool inside = p.place == tls::Place::House;
            const bool selected = static_cast<int>(h) == selectedHousehold &&
                                  static_cast<int>(i) == selectedPerson;
            const float radius = p.role == tls::Role::Child ? kPersonRadius * 0.8f : kPersonRadius;

            if (selected) DrawCircleV(pos, radius + 0.45f, kSelected);
            DrawCircleV({pos.x + 0.15f, pos.y + 0.2f}, radius, Fade(BLACK, 0.3f));
            DrawCircleV(pos, radius, inside ? Fade(RoleColor(p.role), 0.75f) : RoleColor(p.role));
            if (p.sick) DrawCircleV(pos, radius * 0.35f, Color{120, 200, 110, 255});
        }
    }
}

void HouseholdView::DrawLabels(const tls::Simulation& sim, const Camera2D& camera, Font font,
                               int selectedHousehold, int selectedPerson) const {
    const Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    int hoverH = -1;
    int hoverP = -1;
    PersonAt(sim, mouseWorld, hoverH, hoverP);

    for (const auto& [key, pos] : drawn_) {
        const bool selected = key.first == selectedHousehold && key.second == selectedPerson;
        const bool hover = key.first == hoverH && key.second == hoverP;
        if (!selected && !hover) continue;
        const tls::Person& p = sim.Households()[key.first].members[key.second];
        if (!p.alive) continue;

        const Vector2 screen = GetWorldToScreen2D({pos.x, pos.y - 1.4f}, camera);
        const Vector2 dims = MeasureTextEx(font, p.name.c_str(), 17, 1);
        const Vector2 at{screen.x - dims.x * 0.5f, screen.y - dims.y - 4.0f};
        DrawRectangleRounded({at.x - 6, at.y - 2, dims.x + 12, dims.y + 4}, 0.5f, 6, Fade(BLACK, 0.6f));
        DrawTextEx(font, p.name.c_str(), at, 17, 1, selected ? kSelected : RAYWHITE);
    }
}

bool HouseholdView::PersonAt(const tls::Simulation& sim, Vector2 world, int& household, int& person) const {
    float best = 1.4f;  // metres: generous, people are small
    bool found = false;
    for (const auto& [key, pos] : drawn_) {
        if (!sim.Households()[key.first].members[key.second].alive) continue;
        const float d = Vector2Distance(pos, world);
        if (d < best) {
            best = d;
            household = key.first;
            person = key.second;
            found = true;
        }
    }
    return found;
}
