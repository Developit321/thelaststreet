#include "season_look.h"

#include <cmath>

namespace {

// Colours at the middle of each season. Between those points the street
// blends from one season's palette into the next.
struct Palette {
    Color forestFloor, clearing, yard, road, soil, crop, birch;
};

constexpr Palette kSeasons[tls::kSeasonCount] = {
    // Spring: wet, muddy, first green
    {{52, 62, 42, 255}, {110, 122, 74, 255}, {104, 118, 70, 255}, {98, 78, 58, 255},
     {74, 56, 40, 255}, {116, 150, 76, 255}, {132, 156, 96, 255}},
    // Summer: deep green, gold light
    {{46, 64, 40, 255}, {104, 128, 72, 255}, {116, 132, 78, 255}, {122, 100, 72, 255},
     {84, 64, 44, 255}, {84, 130, 58, 255}, {96, 124, 66, 255}},
    // Autumn: gold and orange birch, harvest colours
    {{78, 66, 42, 255}, {148, 130, 78, 255}, {138, 122, 76, 255}, {108, 86, 62, 255},
     {86, 64, 44, 255}, {178, 146, 70, 255}, {212, 152, 52, 255}},
    // Winter (before snow): dead grass, bare birch
    {{60, 62, 56, 255}, {136, 128, 104, 255}, {128, 120, 98, 255}, {102, 90, 78, 255},
     {76, 62, 50, 255}, {120, 110, 80, 255}, {122, 110, 98, 255}},
};

constexpr Color kSnow{232, 236, 242, 255};
constexpr Color kPine{34, 58, 38, 255};
constexpr Color kRoof{78, 70, 66, 255};
constexpr Color kRidge{56, 50, 48, 255};
constexpr Color kWall{122, 92, 62, 255};
constexpr Color kFence{150, 128, 98, 255};

Color Mix(Color a, Color b, float t) {
    if (t <= 0.0f) return a;
    if (t >= 1.0f) return b;
    auto ch = [t](unsigned char x, unsigned char y) {
        return static_cast<unsigned char>(x + (y - x) * t);
    };
    return {ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b), ch(a.a, b.a)};
}

Color Darken(Color c, float amount) {
    return {static_cast<unsigned char>(c.r * (1.0f - amount)),
            static_cast<unsigned char>(c.g * (1.0f - amount)),
            static_cast<unsigned char>(c.b * (1.0f - amount)), c.a};
}

// How full the garden is across the year.
float CropAmount(tls::Season season, float progress) {
    switch (season) {
        case tls::Season::Spring: return 0.15f + 0.6f * progress;
        case tls::Season::Summer: return 1.0f;
        case tls::Season::Autumn: return 1.0f - progress;  // harvested through autumn
        case tls::Season::Winter: return 0.0f;
    }
    return 0.0f;
}

}  // namespace

SceneLook ComputeLook(const tls::Simulation& sim) {
    const tls::Date date = sim.CurrentDate();
    const float progress = sim.SeasonProgress();

    // Position on a loop where each season's palette sits at its midpoint.
    const float pos = static_cast<float>(date.season) + progress - 0.5f;
    const float wrapped = pos < 0.0f ? pos + tls::kSeasonCount : pos;
    const int from = static_cast<int>(std::floor(wrapped)) % tls::kSeasonCount;
    const int to = (from + 1) % tls::kSeasonCount;
    const float t = wrapped - std::floor(wrapped);
    const Palette& a = kSeasons[from];
    const Palette& b = kSeasons[to];

    const float snow = sim.SnowCover();

    SceneLook look{};
    look.snow = snow;
    look.snowing = sim.Snowing() && sim.TemperatureC() < 1.0f;
    look.darkness = 1.0f - sim.Light();

    look.forestFloor = Mix(Mix(a.forestFloor, b.forestFloor, t), kSnow, snow * 0.85f);
    look.clearing = Mix(Mix(a.clearing, b.clearing, t), kSnow, snow);
    look.yard = Mix(Mix(a.yard, b.yard, t), kSnow, snow);
    look.road = Mix(Mix(a.road, b.road, t), kSnow, snow * 0.75f);
    look.rut = Mix(Darken(look.road, 0.2f), Darken(Mix(a.road, b.road, t), 0.1f), snow * 0.6f);
    look.soil = Mix(Mix(a.soil, b.soil, t), kSnow, snow);
    look.crop = Mix(a.crop, b.crop, t);
    look.cropAmount = CropAmount(date.season, progress) * (1.0f - snow);
    look.pine = Mix(kPine, Color{176, 188, 192, 255}, snow * 0.45f);
    look.birch = Mix(Mix(a.birch, b.birch, t), kSnow, snow * 0.55f);
    look.roof = Mix(kRoof, kSnow, snow);
    look.ridge = Mix(kRidge, Color{200, 204, 210, 255}, snow);
    look.wall = kWall;
    look.fence = kFence;
    return look;
}
