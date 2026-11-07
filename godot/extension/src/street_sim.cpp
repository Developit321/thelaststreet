#include "street_sim.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "data_loader.h"

namespace godot {

namespace {

String Str(const std::string& s) { return String::utf8(s.c_str()); }
Rect2 ToRect(const tls::Rect& r) { return Rect2(r.x, r.y, r.w, r.h); }

const char* EventName(tls::EventType t) {
    switch (t) {
        case tls::EventType::NewDay: return "new_day";
        case tls::EventType::NewSeason: return "new_season";
        case tls::EventType::NewYear: return "new_year";
        case tls::EventType::PersonFellSick: return "fell_sick";
        case tls::EventType::PersonRecovered: return "recovered";
        case tls::EventType::PersonDied: return "died";
        case tls::EventType::FoodRanOut: return "food_ran_out";
        case tls::EventType::FirewoodRanOut: return "firewood_ran_out";
        case tls::EventType::HouseholdLost: return "household_lost";
        case tls::EventType::FoodSpoiled: return "food_spoiled";
        case tls::EventType::PlantingDone: return "planting_done";
        case tls::EventType::CropsReady: return "crops_ready";
        case tls::EventType::HarvestDone: return "harvest_done";
        case tls::EventType::CropsLostToFrost: return "frost";
    }
    return "unknown";
}

const char* WarningName(tls::WarningType t) {
    switch (t) {
        case tls::WarningType::FirewoodLow: return "firewood_low";
        case tls::WarningType::FirewoodOut: return "firewood_out";
        case tls::WarningType::FoodLow: return "food_low";
        case tls::WarningType::FoodOut: return "food_out";
        case tls::WarningType::HouseCold: return "house_cold";
        case tls::WarningType::PersonSick: return "person_sick";
        case tls::WarningType::PersonWeak: return "person_weak";
        case tls::WarningType::CropReady: return "crop_ready";
        case tls::WarningType::StorageFull: return "storage_full";
    }
    return "unknown";
}

}  // namespace

bool StreetSim::load(const String& data_dir) {
    const std::string dir = std::string(data_dir.utf8().get_data()) + "/";
    sim_.reset();

    const StreetLoadResult street = LoadStreet(dir + "street.json");
    const CalendarLoadResult calendar = LoadCalendar(dir + "calendar.json");
    const NeedsLoadResult needs = LoadNeeds(dir + "needs.json");
    const WorkLoadResult work = LoadWork(dir + "work.json");
    HouseholdsLoadResult households;
    if (street.ok) households = LoadHouseholds(dir + "households.json", street.street);

    if (!street.ok) error_ = street.error;
    else if (!calendar.ok) error_ = calendar.error;
    else if (!needs.ok) error_ = needs.error;
    else if (!work.ok) error_ = work.error;
    else if (!households.ok) error_ = households.error;
    else error_.clear();
    if (!error_.empty()) return false;

    config_ = {calendar.calendar, calendar.climate, needs.needs, work.work};
    street_ = street.street;
    households_ = households.households;
    restart();
    return true;
}

String StreetSim::get_error() const { return Str(error_); }

void StreetSim::restart() {
    sim_ = std::make_unique<tls::Simulation>(config_, street_, households_);
    sim_->TakeEvents();
}

void StreetSim::advance(int minutes) {
    if (!Ready()) return;
    for (int i = 0; i < minutes; ++i) sim_->Step();
}

Dictionary StreetSim::get_time() const {
    Dictionary d;
    if (!Ready()) return d;
    const tls::Date date = sim_->CurrentDate();
    const int64_t day = sim_->GetCalendar().DayIndex(sim_->Minute());
    const tls::Daylight light = sim_->GetWeather().DaylightOn(day);
    d["minute_total"] = sim_->Minute();
    d["year"] = date.year;
    d["season"] = static_cast<int>(date.season);
    d["season_name"] = tls::SeasonName(date.season);
    d["day"] = date.day;
    d["hour"] = date.Hour();
    d["minute"] = date.Minute();
    d["days_per_season"] = sim_->GetCalendar().DaysPerSeason();
    d["season_progress"] = sim_->SeasonProgress();
    d["temperature"] = sim_->TemperatureC();
    d["snow"] = sim_->SnowCover();
    d["light"] = sim_->Light();
    d["snowing"] = sim_->Snowing() && sim_->TemperatureC() < 1.0f;
    d["sunrise"] = light.sunrise;
    d["sunset"] = light.sunset;
    return d;
}

Dictionary StreetSim::date_at(int64_t minute) const {
    Dictionary d;
    if (!Ready()) return d;
    const tls::Date date = sim_->GetCalendar().ToDate(minute);
    d["year"] = date.year;
    d["season"] = static_cast<int>(date.season);
    d["season_name"] = tls::SeasonName(date.season);
    d["day"] = date.day;
    d["hour"] = date.Hour();
    d["minute"] = date.Minute();
    return d;
}

Dictionary StreetSim::get_street() const {
    Dictionary d;
    if (!Ready()) return d;
    const tls::StreetLayout& s = sim_->Street();
    d["country"] = Str(s.countryName);
    d["street"] = Str(s.streetName);
    d["world"] = ToRect(s.world);
    d["road"] = ToRect(s.road);
    d["pump"] = Vector2(s.pump.x, s.pump.y);
    d["railway_y"] = s.railwayY;
    Array plots;
    for (const tls::Plot& p : s.plots) {
        Dictionary pd;
        pd["id"] = Str(p.id);
        pd["household"] = Str(p.household);
        pd["north"] = p.side == tls::StreetSide::North;
        pd["is_player"] = p.isPlayer;
        pd["bounds"] = ToRect(p.bounds);
        pd["house"] = ToRect(p.house);
        pd["garden"] = ToRect(p.garden);
        pd["woodpile"] = ToRect(p.woodpile);
        plots.append(pd);
    }
    d["plots"] = plots;
    return d;
}

Array StreetSim::get_households() const {
    Array out;
    if (!Ready()) return out;
    const tls::WorkConfig& work = sim_->Work();
    for (const tls::Household& h : sim_->Households()) {
        Dictionary hd;
        hd["id"] = Str(h.id);
        hd["plot"] = Str(h.plotId);
        hd["name"] = Str(h.name);
        hd["is_player"] = h.isPlayer;
        hd["lost"] = h.lost;
        hd["fresh_food"] = h.stores.freshFood;
        hd["preserved_food"] = h.stores.preservedFood;
        hd["firewood"] = h.stores.firewood;
        hd["indoor_c"] = h.indoorC;
        hd["stove_power"] = h.stovePower;

        Dictionary garden;
        garden["planted"] = h.garden.planted;
        garden["growth"] = h.garden.growth;
        garden["care"] = h.garden.care;
        garden["harvested"] = h.garden.harvested;
        garden["expected"] = h.ExpectedHarvest(work);
        garden["in_ground"] = h.CropInGround(work);
        hd["garden"] = garden;

        Array members;
        for (const tls::Person& p : h.members) {
            Dictionary pd;
            pd["name"] = Str(p.name);
            pd["age"] = p.age;
            pd["role"] = static_cast<int>(p.role);
            pd["role_name"] = tls::RoleName(p.role);
            pd["alive"] = p.alive;
            pd["health"] = p.health;
            pd["warmth"] = p.warmth;
            pd["hunger"] = p.hunger;
            pd["morale"] = p.morale;
            pd["sick"] = p.sick;
            pd["place"] = static_cast<int>(p.place);
            pd["activity"] = tls::ActivityName(p.activity);
            pd["job"] = static_cast<int>(p.job);
            pd["order_until"] = p.orderUntil;
            pd["order_place"] = static_cast<int>(p.orderPlace);
            pd["cause"] = tls::DeathCauseName(p.cause);
            pd["died_at"] = p.diedAt;
            members.append(pd);
        }
        hd["members"] = members;
        out.append(hd);
    }
    return out;
}

int StreetSim::player_household() const { return Ready() ? sim_->PlayerHousehold() : -1; }

bool StreetSim::player_lost() const { return Ready() && sim_->PlayerLost(); }

Array StreetSim::get_warnings(int household) const {
    Array out;
    if (!Ready()) return out;
    for (const tls::Warning& w : sim_->Warnings(household)) {
        Dictionary wd;
        wd["type"] = WarningName(w.type);
        wd["person"] = w.person;
        wd["value"] = w.value;
        wd["urgent"] = w.urgent;
        out.append(wd);
    }
    return out;
}

float StreetSim::food_days_left(int household) const {
    return Ready() ? sim_->FoodDaysLeft(household) : 0.0f;
}

float StreetSim::firewood_days_left(int household) const {
    return Ready() ? sim_->FirewoodDaysLeft(household) : 0.0f;
}

Array StreetSim::take_events() {
    Array out;
    if (!Ready()) return out;
    for (const tls::Event& e : sim_->TakeEvents()) {
        Dictionary ed;
        ed["type"] = EventName(e.type);
        ed["minute"] = e.minute;
        ed["household"] = e.household;
        ed["person"] = e.person;
        ed["value"] = e.value;
        out.append(ed);
    }
    return out;
}

void StreetSim::set_job(int household, int person, int job) {
    if (Ready() && job >= 0 && job < tls::kJobCount) sim_->SetJob(household, person, static_cast<tls::Job>(job));
}

void StreetSim::order(int household, int person, int place) {
    if (Ready() && place >= 0 && place <= static_cast<int>(tls::Place::Yard)) {
        sim_->Order(household, person, static_cast<tls::Place>(place));
    }
}

void StreetSim::cancel_order(int household, int person) {
    if (Ready()) sim_->CancelOrder(household, person);
}

void StreetSim::jump_to_season(int season) {
    if (Ready() && season >= 0 && season < tls::kSeasonCount) sim_->JumpToSeason(static_cast<tls::Season>(season));
}

void StreetSim::adjust_stores(int household, float food, float firewood) {
    if (Ready()) sim_->AdjustStores(household, food, firewood);
}

void StreetSim::ripen_garden(int household) {
    if (Ready()) sim_->RipenGarden(household);
}

void StreetSim::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load", "data_dir"), &StreetSim::load);
    ClassDB::bind_method(D_METHOD("get_error"), &StreetSim::get_error);
    ClassDB::bind_method(D_METHOD("restart"), &StreetSim::restart);
    ClassDB::bind_method(D_METHOD("advance", "minutes"), &StreetSim::advance);
    ClassDB::bind_method(D_METHOD("get_time"), &StreetSim::get_time);
    ClassDB::bind_method(D_METHOD("date_at", "minute"), &StreetSim::date_at);
    ClassDB::bind_method(D_METHOD("get_street"), &StreetSim::get_street);
    ClassDB::bind_method(D_METHOD("get_households"), &StreetSim::get_households);
    ClassDB::bind_method(D_METHOD("player_household"), &StreetSim::player_household);
    ClassDB::bind_method(D_METHOD("player_lost"), &StreetSim::player_lost);
    ClassDB::bind_method(D_METHOD("get_warnings", "household"), &StreetSim::get_warnings);
    ClassDB::bind_method(D_METHOD("food_days_left", "household"), &StreetSim::food_days_left);
    ClassDB::bind_method(D_METHOD("firewood_days_left", "household"), &StreetSim::firewood_days_left);
    ClassDB::bind_method(D_METHOD("take_events"), &StreetSim::take_events);
    ClassDB::bind_method(D_METHOD("set_job", "household", "person", "job"), &StreetSim::set_job);
    ClassDB::bind_method(D_METHOD("order", "household", "person", "place"), &StreetSim::order);
    ClassDB::bind_method(D_METHOD("cancel_order", "household", "person"), &StreetSim::cancel_order);
    ClassDB::bind_method(D_METHOD("jump_to_season", "season"), &StreetSim::jump_to_season);
    ClassDB::bind_method(D_METHOD("adjust_stores", "household", "food", "firewood"), &StreetSim::adjust_stores);
    ClassDB::bind_method(D_METHOD("ripen_garden", "household"), &StreetSim::ripen_garden);
}

}  // namespace godot
