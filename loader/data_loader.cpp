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

    int Integer(const json& obj, const char* key, const std::string& where) {
        const json* v = Field(obj, key, where);
        if (!v) return 0;
        if (!v->is_number_integer()) {
            Fail(where + ": '" + key + "' must be a whole number");
            return 0;
        }
        return v->get<int>();
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

// Reads and parses a JSON file. On failure, returns a discarded value and
// fills in the error.
json ReadJson(const std::string& path, std::string& error) {
    std::ifstream file(path);
    if (!file) {
        error = "could not open " + path;
        return json(json::value_t::discarded);
    }
    std::stringstream text;
    text << file.rdbuf();

    json root = json::parse(text.str(), nullptr, /*allow_exceptions=*/false);
    if (root.is_discarded()) error = path + " is not valid JSON";
    return root;
}

}  // namespace

StreetLoadResult LoadStreet(const std::string& path) {
    StreetLoadResult result;

    const json root = ReadJson(path, result.error);
    if (root.is_discarded()) return result;

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

CalendarLoadResult LoadCalendar(const std::string& path) {
    CalendarLoadResult result;

    const json root = ReadJson(path, result.error);
    if (root.is_discarded()) return result;

    Reader r;
    if (const json* c = r.Field(root, "calendar", "calendar file")) {
        result.calendar.startYear = r.Integer(*c, "start_year", "calendar");
        result.calendar.daysPerSeason = r.Integer(*c, "days_per_season", "calendar");
        result.calendar.startHour = r.Integer(*c, "start_hour", "calendar");
    }
    if (const json* c = r.Field(root, "climate", "calendar file")) {
        result.climate.meanC = r.Number(*c, "mean_c", "climate");
        result.climate.seasonalSwingC = r.Number(*c, "seasonal_swing_c", "climate");
        result.climate.dailySwingC = r.Number(*c, "daily_swing_c", "climate");
        result.climate.randomnessC = r.Number(*c, "randomness_c", "climate");
        result.climate.daylightSwingHours = r.Number(*c, "daylight_swing_hours", "climate");
        result.climate.seed = static_cast<uint32_t>(r.Integer(*c, "seed", "climate"));
    }
    if (const json* p = r.Field(root, "pace", "calendar file")) {
        result.pace.realSecondsPerDay = r.Number(*p, "real_seconds_per_day", "pace");
        if (const json* speeds = r.Field(*p, "speeds", "pace")) {
            if (!speeds->is_array() || speeds->empty()) {
                r.Fail("pace: 'speeds' must be a list of numbers");
            } else {
                for (const json& s : *speeds) {
                    if (!s.is_number() || s.get<float>() <= 0.0f) {
                        r.Fail("pace: every speed must be a number above 0");
                        break;
                    }
                    result.pace.speeds.push_back(s.get<float>());
                }
            }
        }
    }

    if (r.error.empty()) {
        if (result.calendar.daysPerSeason < 1) r.Fail("calendar: 'days_per_season' must be at least 1");
        if (result.calendar.startHour < 0 || result.calendar.startHour > 23) {
            r.Fail("calendar: 'start_hour' must be between 0 and 23");
        }
        if (result.pace.realSecondsPerDay <= 0.0f) {
            r.Fail("pace: 'real_seconds_per_day' must be above 0");
        }
    }
    if (!r.error.empty()) {
        result.error = path + ": " + r.error;
        return result;
    }
    result.ok = true;
    return result;
}
