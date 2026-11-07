#include "time_controls.h"

#include "raylib.h"

namespace {
// Never run more than this many minutes in one frame, so a stall (window
// dragged, laptop asleep) doesn't make the game race to catch up.
constexpr int kMaxStepsPerFrame = 2000;
}  // namespace

void TimeControls::SetSpeed(int index) {
    if (index < 0 || index >= static_cast<int>(pace_.speeds.size())) return;
    speedIndex_ = index;
    paused_ = false;
}

void TimeControls::Update(float dt, tls::Simulation& sim) {
    if (IsKeyPressed(KEY_SPACE)) paused_ = !paused_;
    const int speedKeys[] = {KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX};
    for (int i = 0; i < 6; ++i) {
        if (IsKeyPressed(speedKeys[i])) SetSpeed(i);
    }

    if (paused_) {
        pendingMinutes_ = 0.0;
        return;
    }
    const double minutesPerSecond =
        static_cast<double>(tls::kMinutesPerDay) / pace_.realSecondsPerDay * pace_.speeds[speedIndex_];
    pendingMinutes_ += dt * minutesPerSecond;

    int steps = 0;
    while (pendingMinutes_ >= 1.0 && steps < kMaxStepsPerFrame) {
        sim.Step();
        pendingMinutes_ -= 1.0;
        ++steps;
    }
    if (steps == kMaxStepsPerFrame) pendingMinutes_ = 0.0;
}
