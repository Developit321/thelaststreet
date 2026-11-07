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
                plot.woodpile = r.Rect(p, "woodpile", where);
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

NeedsLoadResult LoadNeeds(const std::string& path) {
    NeedsLoadResult result;

    const json root = ReadJson(path, result.error);
    if (root.is_discarded()) return result;

    Reader r;
    tls::NeedsConfig& n = result.needs;
    if (const json* s = r.Field(root, "food", "needs file")) {
        n.adultRationsPerDay = r.Number(*s, "adult_rations_per_day", "food");
        n.childRationsPerDay = r.Number(*s, "child_rations_per_day", "food");
        n.elderRationsPerDay = r.Number(*s, "elder_rations_per_day", "food");
        n.hungerPerDayUnfed = r.Number(*s, "hunger_per_day_unfed", "food");
    }
    if (const json* s = r.Field(root, "house", "needs file")) {
        n.houseHeatLossPerHour = r.Number(*s, "heat_loss_per_hour", "house");
        n.stoveHeatPerHour = r.Number(*s, "stove_heat_per_hour", "house");
        n.stoveBundlesPerHour = r.Number(*s, "stove_bundles_per_hour", "house");
        n.cookingBundlesPerMeal = r.Number(*s, "cooking_bundles_per_meal", "house");
        n.comfortableIndoorC = r.Number(*s, "comfortable_indoor_c", "house");
        n.nightIndoorC = r.Number(*s, "night_indoor_c", "house");
    }
    if (const json* s = r.Field(root, "body", "needs file")) {
        n.comfortableC = r.Number(*s, "comfortable_c", "body");
        n.freezingC = r.Number(*s, "freezing_c", "body");
        n.warmthChangePerHour = r.Number(*s, "warmth_change_per_hour", "body");
        n.coldDamagePerDay = r.Number(*s, "cold_damage_per_day", "body");
        n.hungerDamagePerDay = r.Number(*s, "hunger_damage_per_day", "body");
        n.sicknessDamagePerDay = r.Number(*s, "sickness_damage_per_day", "body");
        n.recoveryPerDay = r.Number(*s, "recovery_per_day", "body");
    }
    if (const json* s = r.Field(root, "sickness", "needs file")) {
        n.sicknessBaseChance = r.Number(*s, "base_chance_per_day", "sickness");
        n.sicknessColdChance = r.Number(*s, "cold_chance_per_day", "sickness");
        n.sicknessRecoverChance = r.Number(*s, "recover_chance_per_day", "sickness");
    }
    if (const json* s = r.Field(root, "frailty", "needs file")) {
        n.childFrailty = r.Number(*s, "child", "frailty");
        n.elderFrailty = r.Number(*s, "elder", "frailty");
    }
    if (const json* s = r.Field(root, "warnings", "needs file")) {
        n.warnDaysLeft = r.Integer(*s, "warn_days_left", "warnings");
    }

    if (r.error.empty() && n.comfortableC <= n.freezingC) {
        r.Fail("body: 'comfortable_c' must be above 'freezing_c'");
    }
    if (!r.error.empty()) {
        result.error = path + ": " + r.error;
        return result;
    }
    result.ok = true;
    return result;
}

HouseholdsLoadResult LoadHouseholds(const std::string& path, const tls::StreetLayout& street) {
    HouseholdsLoadResult result;

    const json root = ReadJson(path, result.error);
    if (root.is_discarded()) return result;

    Reader r;
    const json* list = r.Field(root, "households", "households file");
    if (list && !list->is_array()) r.Fail("'households' must be a list");

    int players = 0;
    if (list && list->is_array()) {
        for (size_t i = 0; i < list->size() && r.error.empty(); ++i) {
            const json& h = list->at(i);
            const std::string where = "households[" + std::to_string(i) + "]";
            tls::Household household;
            household.id = r.String(h, "id", where);
            household.plotId = r.String(h, "plot", where);
            household.name = r.String(h, "name", where);
            household.isPlayer = h.is_object() && h.value("player", false);
            players += household.isPlayer ? 1 : 0;

            if (r.error.empty() && !street.FindPlot(household.plotId)) {
                r.Fail(where + ": there is no plot '" + household.plotId + "' in the street");
            }
            if (const json* s = r.Field(h, "stores", where)) {
                household.stores.food = r.Number(*s, "food", where + ".stores");
                household.stores.firewood = r.Number(*s, "firewood", where + ".stores");
            }
            if (const json* members = r.Field(h, "members", where)) {
                if (!members->is_array() || members->empty()) {
                    r.Fail(where + ": 'members' must be a list with at least one person");
                } else {
                    for (size_t m = 0; m < members->size(); ++m) {
                        const json& pj = members->at(m);
                        const std::string pw = where + ".members[" + std::to_string(m) + "]";
                        tls::Person person;
                        person.name = r.String(pj, "name", pw);
                        person.age = r.Integer(pj, "age", pw);
                        const std::string role = r.String(pj, "role", pw);
                        if (role == "worker") person.role = tls::Role::Worker;
                        else if (role == "homemaker") person.role = tls::Role::Homemaker;
                        else if (role == "child") person.role = tls::Role::Child;
                        else if (role == "elder") person.role = tls::Role::Elder;
                        else r.Fail(pw + ": 'role' must be worker, homemaker, child or elder");
                        household.members.push_back(person);
                    }
                }
            }
            result.households.push_back(household);
        }
    }
    if (r.error.empty() && players != 1) r.Fail("exactly one household must be the player's");

    if (!r.error.empty()) {
        result.error = path + ": " + r.error;
        return result;
    }
    result.ok = true;
    return result;
}
