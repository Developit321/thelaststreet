// The Last Street: gameplay prototype (raylib front end).

#include <algorithm>
#include <string>

#include "data_loader.h"
#include "raylib.h"
#include "street_camera.h"
#include "street_view.h"

namespace {

constexpr Color kBackground{24, 30, 24, 255};
constexpr Color kText{236, 230, 216, 255};
constexpr Color kMuted{176, 168, 150, 255};

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
    const char* candidates[] = {
        "/System/Library/Fonts/Supplemental/Georgia.ttf",
        "/Library/Fonts/Georgia.ttf",
    };
    for (const char* path : candidates) {
        if (FileExists(path)) {
            Font font = LoadFontEx(path, 40, nullptr, 0);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }
    }
    return GetFontDefault();
}

void DrawHud(const tls::StreetLayout& street, const StreetCamera& camera, Font font) {
    const std::string title = street.streetName + ", " + street.countryName;
    DrawRectangle(0, 0, GetScreenWidth(), 64, Fade(BLACK, 0.45f));
    DrawTextEx(font, title.c_str(), {20, 10}, 28, 1, kText);
    DrawTextEx(font, "The Last Street  -  prototype step 1: the street", {20, 40}, 16, 1, kMuted);

    const std::string zoom = TextFormat("%.1f px per metre", camera.PixelsPerMetre());
    const Vector2 zoomSize = MeasureTextEx(font, zoom.c_str(), 16, 1);
    DrawTextEx(font, zoom.c_str(), {GetScreenWidth() - zoomSize.x - 20, 24}, 16, 1, kMuted);

    const char* help = "Move: drag, arrow keys or WASD    Zoom: scroll or + / -    Reset view: R";
    const int h = GetScreenHeight();
    DrawRectangle(0, h - 36, GetScreenWidth(), 36, Fade(BLACK, 0.45f));
    DrawTextEx(font, help, {20, static_cast<float>(h - 27)}, 18, 1, kText);
}

void RunErrorScreen(const std::string& error, Font font) {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(kBackground);
        DrawTextEx(font, "The street data could not be loaded:", {40, 40}, 24, 1, kText);
        DrawTextEx(font, error.c_str(), {40, 80}, 20, 1, Color{230, 140, 120, 255});
        DrawTextEx(font, "Fix the file in data/ and launch again.", {40, 120}, 20, 1, kMuted);
        EndDrawing();
    }
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "The Last Street");
    SetWindowMinSize(800, 500);
    SetExitKey(KEY_NULL);  // Esc will open menus later; close with the window button or Cmd+Q
    SetTargetFPS(60);

    const Font font = LoadUiFont();
    const StreetLoadResult loaded = LoadStreet(FindDataFile("street.json"));
    if (!loaded.ok) {
        TraceLog(LOG_ERROR, "%s", loaded.error.c_str());
        RunErrorScreen(loaded.error, font);
        CloseWindow();
        return 1;
    }
    const tls::StreetLayout& street = loaded.street;

    StreetView view;
    view.Init(street);

    // Start framed on the plots and road rather than the whole forest.
    tls::Rect focus = street.plots.front().bounds;
    for (const tls::Plot& plot : street.plots) {
        const float left = std::min(focus.x, plot.bounds.x);
        const float top = std::min(focus.y, plot.bounds.y);
        const float right = std::max(focus.Right(), plot.bounds.Right());
        const float bottom = std::max(focus.Bottom(), plot.bounds.Bottom());
        focus = {left, top, right - left, bottom - top};
    }
    focus = {focus.x - 6.0f, focus.y - 10.0f, focus.w + 12.0f, focus.h + 20.0f};
    StreetCamera camera;
    camera.Init(street.world, focus);

    while (!WindowShouldClose()) {
        camera.Update(GetFrameTime());

        BeginDrawing();
        ClearBackground(kBackground);
        BeginMode2D(camera.Camera());
        view.DrawWorld(street);
        EndMode2D();
        view.DrawLabels(street, camera.Camera(), font);
        DrawHud(street, camera, font);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
