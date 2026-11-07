#pragma once

#include "raylib.h"
#include "tls/simulation.h"

// The colours of the street at this moment: they follow the seasons
// smoothly and turn white as snow builds up.
struct SceneLook {
    Color forestFloor;
    Color clearing;
    Color yard;
    Color road;
    Color rut;
    Color soil;
    Color crop;
    float cropAmount;  // 0 = bare soil, 1 = full rows
    Color pine;
    Color birch;
    Color roof;
    Color ridge;
    Color wall;
    Color fence;
    float snow;      // 0..1, for anything that wants to know
    float darkness;  // 0 = full day, 1 = deepest night
    bool snowing;
};

SceneLook ComputeLook(const tls::Simulation& sim);
