#pragma once

#include <map>
#include <string>
#include <vector>

#include "raylib.h"
#include "season_look.h"
#include "tls/simulation.h"

// Draws what households are doing: people, woodpiles, chimney smoke and lit
// windows. Visual only.
class HouseholdView {
public:
    // Moves each person's drawn position toward where the simulation says they are.
    void Update(const tls::Simulation& sim, float dt);

    // Inside BeginMode2D.
    void DrawWorld(const tls::Simulation& sim, const SceneLook& look, int selectedHousehold,
                   int selectedPerson) const;

    // After EndMode2D: names over the selected person and anyone under the mouse.
    void DrawLabels(const tls::Simulation& sim, const Camera2D& camera, Font font,
                    int selectedHousehold, int selectedPerson) const;

    // The person under a world position, if any. Returns false if none.
    bool PersonAt(const tls::Simulation& sim, Vector2 world, int& household, int& person) const;

    void Reset() { drawn_.clear(); }

private:
    Vector2 TargetPosition(const tls::Simulation& sim, int household, int person) const;
    const tls::Plot* PlotOf(const tls::Simulation& sim, int household) const;

    // Drawn position for each person, keyed by household index then person index.
    std::map<std::pair<int, int>, Vector2> drawn_;
};
