#pragma once

// World positions are in metres on a flat top-down plane:
// x grows east, y grows south. Unreal maps these onto its own axes later.

namespace tls {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float Right() const { return x + w; }
    float Bottom() const { return y + h; }
    Vec2 Center() const { return {x + w * 0.5f, y + h * 0.5f}; }

    bool Contains(Vec2 p) const {
        return p.x >= x && p.x <= Right() && p.y >= y && p.y <= Bottom();
    }
    bool Contains(const Rect& r) const {
        return r.x >= x && r.Right() <= Right() && r.y >= y && r.Bottom() <= Bottom();
    }
    bool Overlaps(const Rect& r) const {
        return x < r.Right() && r.x < Right() && y < r.Bottom() && r.y < Bottom();
    }
};

}  // namespace tls
