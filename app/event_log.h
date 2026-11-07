#pragma once

#include <cstdint>
#include <deque>
#include <string>

#include "raylib.h"
#include "tls/simulation.h"
#include "ui.h"

// A running record of what happened on the street, newest first.
class EventLog {
public:
    // Turns a simulation event into a line of text, if it's worth showing.
    void Add(const tls::Simulation& sim, const tls::Event& event);
    void Clear() { entries_.clear(); }
    void Draw(const tls::Simulation& sim, Font font, ui::Blockers& blockers) const;

private:
    struct Entry {
        int64_t minute;
        std::string text;
        bool important;
    };
    std::deque<Entry> entries_;
};
