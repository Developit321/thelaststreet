#pragma once

#include "raylib.h"
#include "tls/simulation.h"
#include "ui.h"

// Hidden testing tools, toggled with the ` key (next to 1) or F1.
// Grows with every prototype step.
class DebugPanel {
public:
    void Update();
    bool Open() const { return open_; }

    // Draws the panel, applies any button presses to the simulation and
    // registers its area so the camera ignores clicks on it.
    void Draw(tls::Simulation& sim, Font font, ui::Blockers& blockers);

private:
    bool open_ = false;
};
