// The Last Street: gameplay prototype (raylib front end).

#include <algorithm>
#include <string>
#include <vector>

#include "data_loader.h"
#include "debug_panel.h"
#include "event_log.h"
#include "household_view.h"
#include "panels.h"
#include "raylib.h"
#include "season_look.h"
#include "street_camera.h"
#include "street_view.h"
#include "time_controls.h"
#include "tls/simulation.h"
#include "ui.h"

namespace {

constexpr Color kBackground{24, 30, 24, 255};
constexpr float kTopBarHeight = 72.0f;
constexpr float kBottomBarHeight = 36.0f;

// Finds a data file whether the game is launched from the project folder
// (make run) or by opening the executable in build/ directly.
std::string FindDataFile(const char* name) {
    const std::string fromProject = std::string("data/") + name;
    if (FileExists(fromProject.c_str())) return fromProject;
    const std::string fromBuild = std::string(GetApplicationDirectory()) + "../data/" + name;
    if (FileExists(fromBuild.c_str())) return fromBuild;
    return fromProject;
}

// A period serif font from macOS if available, otherwise raylib's built-in one.
Font LoadUiFont() {
    // Plain ASCII plus the degree sign, en dash and middle dot.
    std::vector<int> codepoints;
    for (int c = 32; c < 127; ++c) codepoints.push_back(c);
    codepoints.insert(codepoints.end(), {0x00B0, 0x2013, 0x00B7});

    const char* candidates[] = {
        "/System/Library/Fonts/Supplemental/Georgia.ttf",
        "/Library/Fonts/Georgia.ttf",
    };
    for (const char* path : candidates) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 40, codepoints.data(), static_cast<int>(codepoints.size()));
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }
    }
    return GetFontDefault();
}

void DrawTopBar(const tls::Simulation& sim, TimeControls& time, Font font, ui::Blockers& blockers) {
    const float w = static_cast<float>(GetScreenWidth());
    const Rectangle bar{0, 0, w, kTopBarHeight};
    blockers.Add(bar);
    DrawRectangleRec(bar, Fade(BLACK, 0.5f));

    // Where and when.
    const tls::StreetLayout& street = sim.Street();
    const tls::Date date = sim.CurrentDate();
    const std::string place = street.streetName + ", " + street.countryName;
    ui::Text(font, place.c_str(), {20, 10}, 26);
    const char* when = TextFormat("Day %d of %s, %d  ·  %02d:%02d", date.day,
                                  tls::SeasonName(date.season), date.year, date.Hour(), date.Minute());
    ui::Text(font, when, {20, 42}, 18, ui::kMuted);

    // Temperature and weather.
    const float temp = sim.TemperatureC();
    const char* weather = sim.Snowing() && temp < 1.0f ? "Snowing" : (temp < 0.0f ? "Freezing" : "");
    const char* tempText = TextFormat("%+.0f°C", temp);
    const float tempX = w * 0.5f - 60.0f;
    ui::Text(font, tempText, {tempX, 12}, 30,
             temp < 0.0f ? Color{170, 200, 236, 255} : Color{236, 210, 160, 255});
    ui::Text(font, weather, {tempX, 46}, 16, ui::kMuted);

    // Speed buttons: pause plus one per speed.
    const std::vector<float>& speeds = time.GetPace().speeds;
    const float bw = 52.0f;
    const float gap = 6.0f;
    float x = w - 20.0f - (speeds.size() + 1) * (bw + gap) + gap;
    if (ui::Button(font, {x, 20, bw, 32}, "Pause", time.Paused())) time.SetPaused(!time.Paused());
    x += bw + gap;
    for (size_t i = 0; i < speeds.size(); ++i) {
        const bool selected = !time.Paused() && time.SpeedIndex() == static_cast<int>(i);
        if (ui::Button(font, {x, 20, bw, 32}, TextFormat("%gx", speeds[i]), selected)) {
            time.SetSpeed(static_cast<int>(i));
        }
        x += bw + gap;
    }

    // How far through the season, as a thin bar along the bottom edge.
    DrawRectangle(0, static_cast<int>(kTopBarHeight) - 3, static_cast<int>(w), 3, Fade(BLACK, 0.6f));
    DrawRectangle(0, static_cast<int>(kTopBarHeight) - 3, static_cast<int>(w * sim.SeasonProgress()), 3,
                  ui::kAccent);
}

void DrawBottomBar(Font font, ui::Blockers& blockers) {
    const float w = static_cast<float>(GetScreenWidth());
    const float h = static_cast<float>(GetScreenHeight());
    const Rectangle bar{0, h - kBottomBarHeight, w, kBottomBarHeight};
    blockers.Add(bar);
    DrawRectangleRec(bar, Fade(BLACK, 0.5f));
    const char* help =
        "Select: click a person, then click your plot to send them     Move: drag, arrows or WASD     "
        "Zoom: scroll or + / -     Reset: R     Pause: Space     Speed: 1-4     Debug: `";
    ui::Text(font, help, {20, h - 27}, 17);
}

void RunErrorScreen(const std::string& error, Font font) {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(kBackground);
        ui::Text(font, "The game data could not be loaded:", {40, 40}, 24);
        ui::Text(font, error.c_str(), {40, 80}, 20, Color{230, 140, 120, 255});
        ui::Text(font, "Fix the file in data/ and launch again.", {40, 120}, 20, ui::kMuted);
        EndDrawing();
    }
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "The Last Street");
    SetWindowMinSize(900, 560);
    SetExitKey(KEY_NULL);  // Esc will open menus later; close with the window button or Cmd+Q
    SetTargetFPS(60);

    const Font font = LoadUiFont();

    const StreetLoadResult street = LoadStreet(FindDataFile("street.json"));
    const CalendarLoadResult calendar = LoadCalendar(FindDataFile("calendar.json"));
    const NeedsLoadResult needs = LoadNeeds(FindDataFile("needs.json"));
    const WorkLoadResult work = LoadWork(FindDataFile("work.json"));
    HouseholdsLoadResult households;
    if (street.ok) households = LoadHouseholds(FindDataFile("households.json"), street.street);

    std::string error;
    if (!street.ok) error = street.error;
    else if (!calendar.ok) error = calendar.error;
    else if (!needs.ok) error = needs.error;
    else if (!work.ok) error = work.error;
    else if (!households.ok) error = households.error;
    if (!error.empty()) {
        TraceLog(LOG_ERROR, "%s", error.c_str());
        RunErrorScreen(error, font);
        CloseWindow();
        return 1;
    }

    const tls::SimConfig config{calendar.calendar, calendar.climate, needs.needs, work.work};
    tls::Simulation sim(config, street.street, households.households);
    TimeControls time(calendar.pace);
    DebugPanel debug;
    HouseholdView householdView;
    EventLog log;
    Selection selection;

    StreetView view;
    view.Init(sim.Street());

    // Start framed on the plots and road rather than the whole forest.
    tls::Rect focus = sim.Street().plots.front().bounds;
    for (const tls::Plot& plot : sim.Street().plots) {
        const float left = std::min(focus.x, plot.bounds.x);
        const float top = std::min(focus.y, plot.bounds.y);
        const float right = std::max(focus.Right(), plot.bounds.Right());
        const float bottom = std::max(focus.Bottom(), plot.bounds.Bottom());
        focus = {left, top, right - left, bottom - top};
    }
    focus = {focus.x - 6.0f, focus.y - 10.0f, focus.w + 12.0f, focus.h + 20.0f};
    StreetCamera camera;
    camera.Init(sim.Street().world, focus);

    // UI areas from the previous frame decide whether the mouse belongs to
    // the camera or to a panel this frame.
    ui::Blockers blockers;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        camera.Update(dt, blockers.Contains(GetMousePosition()));
        debug.Update();
        if (!sim.PlayerLost()) time.Update(dt, sim);
        for (const tls::Event& e : sim.TakeEvents()) log.Add(sim, e);
        householdView.Update(sim, dt);

        // Clicking a person selects them; clicking empty ground clears it.
        if (camera.Clicked()) {
            const Vector2 world = GetScreenToWorld2D(GetMousePosition(), camera.Camera());
            int h = -1;
            int p = -1;
            tls::Place place;
            if (householdView.PersonAt(sim, world, h, p)) {
                selection.household = h;
                selection.person = p;
            } else if (selection.Any() && selection.household == sim.PlayerHousehold() &&
                       sim.Households()[selection.household].members[selection.person].alive &&
                       householdView.PlaceAt(sim, selection.household, world, place)) {
                // With one of your family selected, clicking your plot sends them there.
                sim.Order(selection.household, selection.person, place);
            } else {
                selection.Clear();
            }
        }

        const SceneLook look = ComputeLook(sim);

        BeginDrawing();
        ClearBackground(kBackground);
        BeginMode2D(camera.Camera());
        view.DrawWorld(sim.Street(), look);
        householdView.DrawWorld(sim, look, selection.household, selection.person);
        EndMode2D();
        view.DrawAtmosphere(look);
        view.DrawLabels(sim.Street(), camera.Camera(), font);
        householdView.DrawLabels(sim, camera.Camera(), font, selection.household, selection.person);

        blockers.Clear();
        DrawTopBar(sim, time, font, blockers);
        DrawBottomBar(font, blockers);
        DrawWarnings(sim, font);
        const float panelBottom = DrawHouseholdPanel(sim, font, blockers, selection);
        DrawPersonPanel(sim, font, blockers, selection, panelBottom);
        log.Draw(sim, font, blockers);
        debug.Draw(sim, font, blockers);

        if (sim.PlayerLost() && DrawGameOver(sim, font, blockers)) {
            sim = tls::Simulation(config, street.street, households.households);
            sim.TakeEvents();
            log.Clear();
            selection.Clear();
            householdView.Reset();
            time.SetPaused(false);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
