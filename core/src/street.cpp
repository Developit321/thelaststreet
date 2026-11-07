#include "tls/street.h"

namespace tls {

const Plot* StreetLayout::FindPlot(const std::string& id) const {
    for (const Plot& plot : plots) {
        if (plot.id == id) return &plot;
    }
    return nullptr;
}

const Plot* StreetLayout::PlayerPlot() const {
    for (const Plot& plot : plots) {
        if (plot.isPlayer) return &plot;
    }
    return nullptr;
}

std::string ValidateStreet(const StreetLayout& street) {
    if (street.world.w <= 0 || street.world.h <= 0) return "the world has no size";
    if (!street.world.Contains(street.road)) return "the road lies outside the world";
    if (!street.road.Contains(street.pump)) return "the pump is not on the road";
    if (street.plots.empty()) return "the street has no plots";

    int playerPlots = 0;
    for (size_t i = 0; i < street.plots.size(); ++i) {
        const Plot& plot = street.plots[i];
        const std::string name = "plot '" + plot.id + "'";

        if (plot.id.empty()) return "a plot has no id";
        if (plot.isPlayer) ++playerPlots;
        if (!street.world.Contains(plot.bounds)) return name + " lies outside the world";
        if (plot.bounds.Overlaps(street.road)) return name + " overlaps the road";
        if (!plot.bounds.Contains(plot.house)) return name + ": the house is outside the plot";
        if (!plot.bounds.Contains(plot.garden)) return name + ": the garden is outside the plot";
        if (plot.house.Overlaps(plot.garden)) return name + ": the house overlaps the garden";

        for (size_t j = i + 1; j < street.plots.size(); ++j) {
            const Plot& other = street.plots[j];
            if (plot.id == other.id) return "two plots share the id '" + plot.id + "'";
            if (plot.bounds.Overlaps(other.bounds)) {
                return name + " overlaps plot '" + other.id + "'";
            }
        }
    }
    if (playerPlots != 1) return "the street needs exactly one player plot";
    return {};
}

}  // namespace tls
