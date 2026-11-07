#include "debug_panel.h"

void DebugPanel::Update() {
    if (IsKeyPressed(KEY_GRAVE) || IsKeyPressed(KEY_F1)) open_ = !open_;
}

void DebugPanel::Draw(tls::Simulation& sim, Font font, ui::Blockers& blockers) {
    if (!open_) return;

    const float width = 300.0f;
    const Rectangle panel{GetScreenWidth() - width - 16.0f, 84.0f, width, 560.0f};
    blockers.Add(panel);
    DrawRectangleRounded(panel, 0.04f, 8, ui::kPanel);

    float x = panel.x + 16.0f;
    float y = panel.y + 14.0f;
    ui::Text(font, "Debug", {x, y}, 22, ui::kAccent);
    y += 34.0f;

    ui::Text(font, "Jump to the start of", {x, y}, 16, ui::kMuted);
    y += 24.0f;
    const tls::Season seasons[] = {tls::Season::Spring, tls::Season::Summer,
                                   tls::Season::Autumn, tls::Season::Winter};
    const float buttonW = (width - 32.0f - 3 * 6.0f) / 4.0f;
    for (int i = 0; i < 4; ++i) {
        const Rectangle r{x + i * (buttonW + 6.0f), y, buttonW, 30.0f};
        const bool current = sim.CurrentDate().season == seasons[i];
        if (ui::Button(font, r, tls::SeasonName(seasons[i]), current)) sim.JumpToSeason(seasons[i]);
    }
    y += 46.0f;

    // Your household's stores.
    const int home = sim.PlayerHousehold();
    if (home >= 0) {
        struct StoreRow {
            const char* label;
            bool food;
        };
        for (const StoreRow& row : {StoreRow{"Food", true}, StoreRow{"Firewood", false}}) {
            ui::Text(font, row.label, {x, y + 6}, 16, ui::kMuted);
            const float bx = x + 84.0f;
            const float bw = 54.0f;
            const tls::Stores& s = sim.Households()[home].stores;
            const float current = row.food ? s.food : s.firewood;
            float change = 0.0f;
            if (ui::Button(font, {bx, y, bw, 28}, "-10")) change = -10.0f;
            if (ui::Button(font, {bx + bw + 6, y, bw, 28}, "+10")) change = 10.0f;
            if (ui::Button(font, {bx + 2 * (bw + 6), y, bw, 28}, "Empty")) change = -current;
            if (change != 0.0f) {
                sim.AdjustStores(home, row.food ? change : 0.0f, row.food ? 0.0f : change);
            }
            y += 36.0f;
        }
        y += 6.0f;
    }

    // The hidden numbers behind the weather.
    const tls::Weather& weather = sim.GetWeather();
    const int64_t day = sim.GetCalendar().DayIndex(sim.Minute());
    const tls::Daylight daylight = weather.DaylightOn(day);
    auto row = [&](const char* label, const char* value) {
        ui::Text(font, label, {x, y}, 16, ui::kMuted);
        const Vector2 dims = MeasureTextEx(font, value, 16, 1);
        ui::Text(font, value, {panel.x + width - 16.0f - dims.x, y}, 16);
        y += 24.0f;
    };
    row("Game minute", TextFormat("%lld", static_cast<long long>(sim.Minute())));
    row("Day number", TextFormat("%lld", static_cast<long long>(day + 1)));
    row("Temperature now", TextFormat("%.1f C", sim.TemperatureC()));
    row("Today's average", TextFormat("%.1f C", weather.DailyMeanC(day)));
    row("Snowfall today", weather.Snowfall(day) ? "yes" : "no");
    row("Snow cover", TextFormat("%.0f%%", sim.SnowCover() * 100.0f));
    row("Daylight", TextFormat("%.0f%%", sim.Light() * 100.0f));
    row("Sunrise / sunset", TextFormat("%02d:%02d / %02d:%02d",
                                       static_cast<int>(daylight.sunrise),
                                       static_cast<int>(daylight.sunrise * 60) % 60,
                                       static_cast<int>(daylight.sunset),
                                       static_cast<int>(daylight.sunset * 60) % 60));
    row("Season progress", TextFormat("%.0f%%", sim.SeasonProgress() * 100.0f));

    // The hidden numbers behind your household.
    if (home >= 0) {
        const tls::Household& h = sim.Households()[home];
        y += 6.0f;
        row("Indoor temperature", TextFormat("%.1f C", h.indoorC));
        row("Stove burning", TextFormat("%.0f%%", h.stovePower * 100.0f));
        row("Firewood used per day", TextFormat("%.1f", h.firewoodPerDay));
        row("Food needed per day", TextFormat("%.1f", h.FoodNeededPerDay(sim.Needs())));
    }
}
