#include "data_loader.h"

#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"

using nlohmann::json;

namespace {

// Small readers that report problems instead of throwing, so a typo in a
// data file shows up as a clear message rather than a crash.
struct Reader {
    std::string error;

    const json* Field(const json& obj, const char* key, const std::string& where) {
        if (!obj.is_object() || !obj.contains(key)) {
            Fail(where + ": missing '" + key + "'");
            return nullptr;
        }
        return &obj.at(key);
    }

    float Number(const json& obj, const char* key, const std::string& where) {
        const json* v = Field(obj, key, where);
        if (!v) return 0.0f;
        if (!v->is_number()) {
            Fail(where + ": '" + key + "' must be a number");
            return 0.0f;
        }
        return v->get<float>();
    }

    std::string String(const json& obj, const char* key, const std::string& where) {
        const json* v = Field(obj, key, where);
        if (!v) return {};
        if (!v->is_string()) {
            Fail(where + ": '" + key + "' must be text");
            return {};
        }
        return v->get<std::string>();
    }

    tls::Rect Rect(const json& obj, const char* key, const std::string& where) {
        const json* v = Field(obj, key, where);
        if (!v) return {};
        const std::string inner = where + "." + key;
        return {Number(*v, "x", inner), Number(*v, "y", inner),
                Number(*v, "w", inner), Number(*v, "h", inner)};
    }

    tls::Vec2 Point(const json& obj, const char* key, const std::string& where) {
        const json* v = Field(obj, key, where);
        if (!v) return {};
        const std::string inner = where + "." + key;
        return {Number(*v, "x", inner), Number(*v, "y", inner)};
    }

    void Fail(const std::string& message) {
        if (error.empty()) error = message;
    }
};

}  // namespace

StreetLoadResult LoadStreet(const std::string& path) {
    StreetLoadResult result;

    std::ifstream file(path);
    if (!file) {
        result.error = "could not open " + path;
        return result;
    }
    std::stringstream text;
    text << file.rdbuf();

    const json root = json::parse(text.str(), nullptr, /*allow_exceptions=*/false);
    if (root.is_discarded()) {
        result.error = path + " is not valid JSON";
        return result;
    }

    Reader r;
    tls::StreetLayout& s = result.street;
    s.countryName = r.String(root, "country", "street");
    s.streetName = r.String(root, "street", "street");
    s.world = r.Rect(root, "world", "street");
    s.road = r.Rect(root, "road", "street");
    s.pump = r.Point(root, "pump", "street");
    s.railwayY = r.Number(root, "railway_y", "street");

    if (const json* plots = r.Field(root, "plots", "street")) {
        if (!plots->is_array()) r.Fail("street: 'plots' must be a list");
        else {
            for (size_t i = 0; i < plots->size(); ++i) {
                const json& p = plots->at(i);
                const std::string where = "plots[" + std::to_string(i) + "]";
                tls::Plot plot;
                plot.id = r.String(p, "id", where);
                plot.household = r.String(p, "household", where);
                plot.isPlayer = p.is_object() && p.value("player", false);

                const std::string side = r.String(p, "side", where);
                if (side == "north") plot.side = tls::StreetSide::North;
                else if (side == "south") plot.side = tls::StreetSide::South;
                else r.Fail(where + ": 'side' must be \"north\" or \"south\"");

                plot.bounds = r.Rect(p, "bounds", where);
                plot.house = r.Rect(p, "house", where);
                plot.garden = r.Rect(p, "garden", where);
                s.plots.push_back(plot);
            }
        }
    }

    if (!r.error.empty()) {
        result.error = path + ": " + r.error;
        return result;
    }
    const std::string problem = tls::ValidateStreet(s);
    if (!problem.empty()) {
        result.error = path + ": " + problem;
        return result;
    }
    result.ok = true;
    return result;
}
