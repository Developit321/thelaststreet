#pragma once

#include "raylib.h"
#include "tls/geometry.h"

// Top-down camera over the street. World units are metres, so the raylib
// zoom value is "pixels per metre".
class StreetCamera {
public:
    void Init(tls::Rect world, tls::Rect focus);
    // mouseOnUi: the cursor is over a panel, so clicks and scrolls belong to it.
    void Update(float dt, bool mouseOnUi);
    void Reset();

    const Camera2D& Camera() const { return camera_; }
    float PixelsPerMetre() const { return camera_.zoom; }

private:
    void ZoomAround(Vector2 screenPoint, float factor);
    void Clamp();
    float FitZoom(tls::Rect area) const;

    Camera2D camera_{};
    tls::Rect world_{};
    tls::Rect focus_{};
    float minZoom_ = 1.0f;
    float maxZoom_ = 40.0f;

    bool dragging_ = false;
    bool dragStarted_ = false;
    Vector2 dragStart_{};
};
