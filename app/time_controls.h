#pragma once

#include "data_loader.h"
#include "tls/simulation.h"

// Turns real time into simulation steps, at the speed the player picked.
class TimeControls {
public:
    explicit TimeControls(const Pace& pace) : pace_(pace) {}

    // Reads the speed keys and advances the simulation by however many
    // minutes have passed.
    void Update(float dt, tls::Simulation& sim);

    bool Paused() const { return paused_; }
    void SetPaused(bool paused) { paused_ = paused; }
    int SpeedIndex() const { return speedIndex_; }
    void SetSpeed(int index);
    const Pace& GetPace() const { return pace_; }

private:
    Pace pace_;
    bool paused_ = false;
    int speedIndex_ = 0;
    double pendingMinutes_ = 0.0;
};
