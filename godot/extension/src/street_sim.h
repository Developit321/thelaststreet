#pragma once

// StreetSim: the bridge between Godot and the simulation in core/.
//
// Godot scripts only ever talk to the game through this class: they advance
// time, read the state as plain dictionaries and arrays, and send the
// player's commands. No game rules live on the Godot side.

#include <memory>
#include <string>
#include <vector>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include "tls/simulation.h"

namespace godot {

class StreetSim : public RefCounted {
    GDCLASS(StreetSim, RefCounted)

public:
    // Loads everything in data_dir and starts a new game. Returns false and
    // sets get_error() if any file is missing or wrong.
    bool load(const String& data_dir);
    String get_error() const;
    // Starts the same game again from the beginning.
    void restart();

    // Moves the world forward by a number of in-game minutes.
    void advance(int minutes);

    Dictionary get_time() const;
    // The calendar date of any game minute: year, season, day, hour, minute.
    Dictionary date_at(int64_t minute) const;
    Dictionary get_street() const;
    Array get_households() const;
    int player_household() const;
    bool player_lost() const;
    Array get_warnings(int household) const;
    float food_days_left(int household) const;
    float firewood_days_left(int household) const;
    // Everything that happened since the last call.
    Array take_events();

    // Player commands.
    void set_job(int household, int person, int job);
    void order(int household, int person, int place);
    void cancel_order(int household, int person);

    // Debug commands.
    void jump_to_season(int season);
    void adjust_stores(int household, float food, float firewood);
    void ripen_garden(int household);

protected:
    static void _bind_methods();

private:
    bool Ready() const { return sim_ != nullptr; }

    std::unique_ptr<tls::Simulation> sim_;
    tls::SimConfig config_;
    tls::StreetLayout street_;
    std::vector<tls::Household> households_;
    std::string error_;
};

}  // namespace godot
