#pragma once

#include <string>
#include <vector>

#include "tls/geometry.h"

namespace tls {

enum class StreetSide { North, South };

// One household's land: the house, its yard and its garden.
struct Plot {
    std::string id;
    std::string household;  // display name, e.g. "The Petrovs"
    StreetSide side = StreetSide::North;
    bool isPlayer = false;
    Rect bounds;
    Rect house;
    Rect garden;
};

// The fixed layout of the street. Everything the simulation later tracks
// (people, stores, buildings) refers back to these places.
struct StreetLayout {
    std::string countryName;
    std::string streetName;
    Rect world;       // everything that exists, including the forest edge
    Rect road;        // the street itself
    Vec2 pump;        // the shared water pump
    float railwayY = 0.0f;
    std::vector<Plot> plots;

    const Plot* FindPlot(const std::string& id) const;
    const Plot* PlayerPlot() const;
};

// Checks the layout makes sense. Returns an empty string when it does,
// otherwise a description of the first problem found.
std::string ValidateStreet(const StreetLayout& street);

}  // namespace tls
