#include "street_view.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

namespace {

constexpr Color kChimney{132, 70, 54, 255};
constexpr Color kShadow{0, 0, 0, 60};
constexpr Color kRail{90, 90, 96, 255};
constexpr Color kSleeper{84, 66, 48, 255};
constexpr Color kPump{70, 86, 104, 255};
constexpr Color kPumpBase{140, 136, 128, 255};
constexpr Color kPlayerHighlight{226, 186, 96, 255};
constexpr Color kNight{10, 16, 40, 255};

constexpr float kFenceThickness = 0.3f;
constexpr float kGateWidth = 3.0f;

Rectangle ToRay(const tls::Rect& r) { return {r.x, r.y, r.w, r.h}; }

tls::Rect Inflate(const tls::Rect& r, float by) {
    return {r.x - by, r.y - by, r.w + by * 2.0f, r.h + by * 2.0f};
}

}  // namespace

void StreetView::Init(const tls::StreetLayout& street) {
    // The street sits in a clearing that covers all the plots, plus a margin.
    tls::Rect area = street.plots.front().bounds;
    for (const tls::Plot& plot : street.plots) {
        const float left = std::min(area.x, plot.bounds.x);
        const float top = std::min(area.y, plot.bounds.y);
        const float right = std::max(area.Right(), plot.bounds.Right());
        const float bottom = std::max(area.Bottom(), plot.bounds.Bottom());
        area = {left, top, right - left, bottom - top};
    }
    clearing_ = Inflate(area, 8.0f);

    // Scatter the forest around the clearing. A fixed seed keeps it the
    // same every launch.
    std::mt19937 rng(1900);
    std::uniform_real_distribution<float> x(street.world.x, street.world.Right());
    std::uniform_real_distribution<float> y(street.world.y, street.world.Bottom());
    std::uniform_real_distribution<float> radius(1.4f, 3.0f);
    std::uniform_int_distribution<int> kind(0, 9);
    const tls::Rect road = Inflate(street.road, 2.0f);

    for (int attempt = 0; attempt < 2600; ++attempt) {
        const tls::Vec2 p{x(rng), y(rng)};
        if (clearing_.Contains(p) || road.Contains(p)) continue;
        if (p.y > street.railwayY - 4.0f && p.y < street.railwayY + 4.0f) continue;
        // Mostly dark pine, some lighter birch.
        trees_.push_back({{p.x, p.y}, radius(rng), kind(rng) < 3});
    }
    // Draw from back (north) to front so overlapping crowns look right.
    std::sort(trees_.begin(), trees_.end(),
              [](const Tree& a, const Tree& b) { return a.pos.y < b.pos.y; });
}

void StreetView::DrawWorld(const tls::StreetLayout& street, const SceneLook& look) const {
    DrawRectangleRec(ToRay(street.world), look.forestFloor);
    DrawRectangleRec(ToRay(clearing_), look.clearing);

    // Railway in the distance: two rails on sleepers.
    for (float sx = street.world.x; sx < street.world.Right(); sx += 1.6f) {
        DrawRectangleRec({sx, street.railwayY - 1.2f, 0.5f, 2.4f}, kSleeper);
    }
    DrawLineEx({street.world.x, street.railwayY - 0.7f},
               {street.world.Right(), street.railwayY - 0.7f}, 0.18f, kRail);
    DrawLineEx({street.world.x, street.railwayY + 0.7f},
               {street.world.Right(), street.railwayY + 0.7f}, 0.18f, kRail);

    // The road, with cart ruts.
    DrawRectangleRec(ToRay(street.road), look.road);
    const float rut1 = street.road.y + street.road.h * 0.3f;
    const float rut2 = street.road.y + street.road.h * 0.7f;
    DrawLineEx({street.road.x, rut1}, {street.road.Right(), rut1}, 0.5f, look.rut);
    DrawLineEx({street.road.x, rut2}, {street.road.Right(), rut2}, 0.5f, look.rut);

    for (const tls::Plot& plot : street.plots) DrawPlot(plot, look);

    // The shared pump on its stone base.
    DrawRectangleRec({street.pump.x - 1.2f, street.pump.y - 1.2f, 2.4f, 2.4f}, kPumpBase);
    DrawCircleV({street.pump.x, street.pump.y}, 0.6f, kPump);

    for (const Tree& tree : trees_) {
        const Color crown = tree.birch ? look.birch : look.pine;
        DrawCircleV({tree.pos.x + 0.5f, tree.pos.y + 0.6f}, tree.radius, kShadow);
        DrawCircleV(tree.pos, tree.radius, crown);
        DrawCircleV({tree.pos.x - tree.radius * 0.25f, tree.pos.y - tree.radius * 0.25f},
                    tree.radius * 0.45f, Fade(WHITE, 0.06f + look.snow * 0.12f));
    }
}

void StreetView::DrawPlot(const tls::Plot& plot, const SceneLook& look) const {
    const tls::Rect& b = plot.bounds;
    DrawRectangleRec(ToRay(b), look.yard);

    // Garden: soil with crop rows that grow and get harvested with the seasons.
    DrawRectangleRec(ToRay(plot.garden), look.soil);
    if (look.cropAmount > 0.02f) {
        const float width = 0.15f + 0.55f * look.cropAmount;
        for (float ry = plot.garden.y + 1.0f; ry < plot.garden.Bottom() - 0.5f; ry += 1.5f) {
            DrawLineEx({plot.garden.x + 0.6f, ry}, {plot.garden.Right() - 0.6f, ry}, width,
                       look.crop);
        }
    }

    // Fence on all four sides, with a gate gap on the side facing the road.
    auto fence = [&look](Vector2 a, Vector2 c) { DrawLineEx(a, c, kFenceThickness, look.fence); };
    const float gateX = plot.house.Center().x;
    const float gateLeft = gateX - kGateWidth * 0.5f;
    const float gateRight = gateX + kGateWidth * 0.5f;
    const bool frontIsBottom = plot.side == tls::StreetSide::North;
    const float frontY = frontIsBottom ? b.Bottom() : b.y;
    const float backY = frontIsBottom ? b.y : b.Bottom();
    fence({b.x, backY}, {b.Right(), backY});
    fence({b.x, b.y}, {b.x, b.Bottom()});
    fence({b.Right(), b.y}, {b.Right(), b.Bottom()});
    fence({b.x, frontY}, {gateLeft, frontY});
    fence({gateRight, frontY}, {b.Right(), frontY});

    // House: shadow, walls, roof with its ridge running along the long side,
    // and a chimney.
    const tls::Rect& h = plot.house;
    DrawRectangleRec({h.x + 0.7f, h.y + 0.8f, h.w, h.h}, kShadow);
    DrawRectangleRec(ToRay(h), look.wall);
    const tls::Rect roof = Inflate(h, -0.4f);
    DrawRectangleRec(ToRay(roof), look.roof);
    if (h.w >= h.h) {
        DrawLineEx({roof.x, roof.Center().y}, {roof.Right(), roof.Center().y}, 0.35f, look.ridge);
    } else {
        DrawLineEx({roof.Center().x, roof.y}, {roof.Center().x, roof.Bottom()}, 0.35f, look.ridge);
    }
    DrawRectangleRec({h.x + h.w * 0.72f, h.y + h.h * 0.22f, 1.1f, 1.1f}, kChimney);

    if (plot.isPlayer) {
        DrawRectangleLinesEx(ToRay(Inflate(b, 0.8f)), 0.35f, kPlayerHighlight);
    }
}

void StreetView::DrawAtmosphere(const SceneLook& look) const {
    const int w = GetScreenWidth();
    const int h = GetScreenHeight();

    if (look.darkness > 0.01f) {
        DrawRectangle(0, 0, w, h, Fade(kNight, look.darkness * 0.6f));
    }

    if (look.snowing) {
        // Flakes drift down and sideways; positions come from a fixed pattern
        // moved by real time, so they keep falling even while paused.
        const float t = static_cast<float>(GetTime());
        for (int i = 0; i < 260; ++i) {
            const float seedX = std::fmod(i * 97.13f, 1.0f) + std::fmod(i * 0.618f, 1.0f);
            const float speed = 40.0f + std::fmod(i * 13.7f, 50.0f);
            const float fx = std::fmod(seedX * w + std::sin(t * 0.8f + i) * 18.0f + t * 12.0f, static_cast<float>(w));
            const float fy = std::fmod(i * 53.0f + t * speed, static_cast<float>(h));
            const float size = 1.2f + std::fmod(i * 0.37f, 1.6f);
            DrawCircleV({fx < 0 ? fx + w : fx, fy}, size, Fade(WHITE, 0.75f));
        }
    }
}

void StreetView::DrawLabels(const tls::StreetLayout& street, const Camera2D& camera,
                            Font font) const {
    const float size = 20.0f;
    const float spacing = 1.0f;

    for (const tls::Plot& plot : street.plots) {
        const std::string text = plot.isPlayer ? plot.household + " (you)" : plot.household;
        // Label just outside the back fence, so it never covers the house.
        const bool north = plot.side == tls::StreetSide::North;
        const tls::Vec2 anchor{plot.bounds.Center().x, north ? plot.bounds.y : plot.bounds.Bottom()};
        Vector2 screen = GetWorldToScreen2D({anchor.x, anchor.y}, camera);

        const Vector2 dims = MeasureTextEx(font, text.c_str(), size, spacing);
        screen.x -= dims.x * 0.5f;
        screen.y += north ? -dims.y - 8.0f : 8.0f;

        const Rectangle pill{screen.x - 8.0f, screen.y - 3.0f, dims.x + 16.0f, dims.y + 6.0f};
        DrawRectangleRounded(pill, 0.5f, 6, Fade(BLACK, 0.55f));
        const Color color = plot.isPlayer ? kPlayerHighlight : RAYWHITE;
        DrawTextEx(font, text.c_str(), screen, size, spacing, color);
    }
}
