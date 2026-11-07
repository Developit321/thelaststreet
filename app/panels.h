#pragma once

#include "raylib.h"
#include "tls/simulation.h"
#include "ui.h"

struct Selection {
    int household = -1;
    int person = -1;
    bool Any() const { return household >= 0 && person >= 0; }
    void Clear() { household = person = -1; }
};

// Your household at a glance: stores, the house and the family. Clicking a
// name selects that person. Returns the bottom edge of the panel.
float DrawHouseholdPanel(const tls::Simulation& sim, Font font, ui::Blockers& blockers,
                         Selection& selection);

// Details for the selected person, placed below the household panel.
void DrawPersonPanel(const tls::Simulation& sim, Font font, ui::Blockers& blockers,
                     Selection& selection, float top);

// Warnings strip under the top bar.
void DrawWarnings(const tls::Simulation& sim, Font font);

// Shown when the whole household has died. Returns true when the player
// asks to start again.
bool DrawGameOver(const tls::Simulation& sim, Font font, ui::Blockers& blockers);
