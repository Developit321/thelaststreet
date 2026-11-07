#pragma once

#include <vector>

#include "raylib.h"

// Tiny immediate-mode UI helpers for the prototype window.

namespace ui {

constexpr Color kText{236, 230, 216, 255};
constexpr Color kMuted{176, 168, 150, 255};
constexpr Color kAccent{226, 186, 96, 255};
constexpr Color kPanel{16, 18, 16, 200};

// Areas covered by UI this frame, so the camera ignores clicks and scrolls
// that land on them.
class Blockers {
public:
    void Clear() { rects_.clear(); }
    void Add(Rectangle r) { rects_.push_back(r); }
    bool Contains(Vector2 p) const;

private:
    std::vector<Rectangle> rects_;
};

// Draws a button and returns true on the frame it is clicked.
bool Button(Font font, Rectangle r, const char* label, bool selected = false);

void Text(Font font, const char* text, Vector2 pos, float size, Color color = kText);

}  // namespace ui
