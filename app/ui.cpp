#include "ui.h"

namespace ui {

bool Blockers::Contains(Vector2 p) const {
    for (const Rectangle& r : rects_) {
        if (CheckCollisionPointRec(p, r)) return true;
    }
    return false;
}

bool Button(Font font, Rectangle r, const char* label, bool selected) {
    const bool hover = CheckCollisionPointRec(GetMousePosition(), r);
    Color fill = selected ? Color{120, 96, 52, 255} : Color{44, 46, 42, 255};
    if (hover) fill = selected ? Color{140, 112, 60, 255} : Color{64, 66, 60, 255};
    DrawRectangleRounded(r, 0.25f, 6, fill);
    if (selected) DrawRectangleRoundedLinesEx(r, 0.25f, 6, 1.5f, kAccent);

    const float size = 16.0f;
    const Vector2 dims = MeasureTextEx(font, label, size, 1);
    DrawTextEx(font, label, {r.x + (r.width - dims.x) * 0.5f, r.y + (r.height - dims.y) * 0.5f},
               size, 1, selected ? kText : kMuted);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void Text(Font font, const char* text, Vector2 pos, float size, Color color) {
    DrawTextEx(font, text, pos, size, 1, color);
}

}  // namespace ui
