#pragma once

#include <vector>

#include "raylib.h"
#include "tls/street.h"

// Draws the street from above. Visual only: nothing here affects the game.
class StreetView {
public:
    void Init(const tls::StreetLayout& street);

    // Call inside BeginMode2D: ground, forest, railway, road, plots.
    void DrawWorld(const tls::StreetLayout& street) const;

    // Call after EndMode2D: household names, which stay readable at any zoom.
    void DrawLabels(const tls::StreetLayout& street, const Camera2D& camera, Font font) const;

private:
    struct Tree {
        Vector2 pos;
        float radius;
        Color color;
    };

    void DrawPlot(const tls::Plot& plot) const;

    std::vector<Tree> trees_;
    tls::Rect clearing_{};
};
