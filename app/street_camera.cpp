#include "street_camera.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kPanSpeedPixels = 700.0f;  // keyboard pan, in screen pixels per second
constexpr float kWheelStep = 0.12f;        // zoom change per scroll notch
constexpr float kKeyZoomPerSecond = 1.8f;
constexpr float kDragThreshold = 4.0f;     // pixels before a click counts as a drag
}  // namespace

void StreetCamera::Init(tls::Rect world, tls::Rect focus) {
    world_ = world;
    focus_ = focus;
    Reset();
}

float StreetCamera::FitZoom(tls::Rect area) const {
    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());
    return std::min(sw / area.w, sh / area.h);
}

void StreetCamera::Reset() {
    camera_.zoom = FitZoom(focus_) * 0.95f;
    const tls::Vec2 c = focus_.Center();
    camera_.target = {c.x, c.y};
    camera_.offset = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    camera_.rotation = 0.0f;
}

void StreetCamera::ZoomAround(Vector2 screenPoint, float factor) {
    const Vector2 before = GetScreenToWorld2D(screenPoint, camera_);
    camera_.zoom = std::clamp(camera_.zoom * factor, minZoom_, maxZoom_);
    const Vector2 after = GetScreenToWorld2D(screenPoint, camera_);
    camera_.target.x += before.x - after.x;
    camera_.target.y += before.y - after.y;
}

void StreetCamera::Update(float dt, bool mouseOnUi) {
    // Keep the view centred if the window was resized.
    camera_.offset = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    minZoom_ = FitZoom(world_);

    // Keyboard pan: arrows or WASD.
    Vector2 pan{0.0f, 0.0f};
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) pan.x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) pan.x += 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) pan.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) pan.y += 1.0f;
    camera_.target.x += pan.x * kPanSpeedPixels * dt / camera_.zoom;
    camera_.target.y += pan.y * kPanSpeedPixels * dt / camera_.zoom;

    // Mouse drag with any button. A left click only becomes a drag once it
    // moves a few pixels, so left clicks stay free for selecting later.
    const Vector2 mouse = GetMousePosition();
    const bool anyPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
                            IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
                            IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
    const bool anyDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ||
                         IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
                         IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    if (anyPressed && !mouseOnUi) {
        dragging_ = true;
        dragStarted_ = false;
        dragStart_ = mouse;
    }
    if (dragging_ && !anyDown) dragging_ = false;
    if (dragging_) {
        const float moved = std::hypot(mouse.x - dragStart_.x, mouse.y - dragStart_.y);
        if (moved > kDragThreshold) dragStarted_ = true;
        if (dragStarted_) {
            const Vector2 delta = GetMouseDelta();
            camera_.target.x -= delta.x / camera_.zoom;
            camera_.target.y -= delta.y / camera_.zoom;
        }
    }

    // Zoom: scroll wheel / two-finger scroll toward the cursor, +/- keys
    // toward the centre of the screen.
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f && !mouseOnUi) ZoomAround(mouse, std::exp(wheel * kWheelStep));
    float keyZoom = 0.0f;
    if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD)) keyZoom += 1.0f;
    if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) keyZoom -= 1.0f;
    if (keyZoom != 0.0f) {
        ZoomAround(camera_.offset, std::exp(keyZoom * kKeyZoomPerSecond * dt));
    }

    if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_HOME)) Reset();

    Clamp();
}

void StreetCamera::Clamp() {
    camera_.zoom = std::clamp(camera_.zoom, minZoom_, maxZoom_);
    // Keep the centre of the view inside the world.
    camera_.target.x = std::clamp(camera_.target.x, world_.x, world_.Right());
    camera_.target.y = std::clamp(camera_.target.y, world_.y, world_.Bottom());
}
