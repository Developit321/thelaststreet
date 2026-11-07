#include "tls/weather.h"

#include <algorithm>
#include <cmath>

namespace tls {

namespace {

constexpr float kTwoPi = 6.28318530718f;
constexpr int kSnowLookbackDays = 40;

uint32_t Hash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

}  // namespace

Weather::Weather(const Calendar& calendar, const ClimateConfig& climate)
    : calendar_(calendar), climate_(climate) {}

float Weather::Noise(int64_t dayIndex, uint32_t channel) const {
    const uint32_t h = Hash(static_cast<uint32_t>(dayIndex) * 2654435761U ^
                            Hash(climate_.seed + channel * 0x9e3779b9U));
    return static_cast<float>(h) / 4294967295.0f * 2.0f - 1.0f;
}

float Weather::YearPhase(int64_t dayIndex, float peakDay) const {
    const float perYear = static_cast<float>(calendar_.DaysPerYear());
    const float day = static_cast<float>(calendar_.DayOfYear(dayIndex)) + 0.5f;
    return std::cos(kTwoPi * (day - peakDay) / perYear);
}

float Weather::DailyMeanC(int64_t dayIndex) const {
    // Warmest in the middle of summer, coldest in the middle of winter.
    const float season = static_cast<float>(calendar_.DaysPerSeason());
    const float midsummer = season * 1.5f;
    const float seasonal = climate_.seasonalSwingC * YearPhase(dayIndex, midsummer);
    // Blend with yesterday's noise so the weather drifts instead of jumping.
    const float noise = Noise(dayIndex, 0) * 0.65f + Noise(dayIndex - 1, 0) * 0.35f;
    return climate_.meanC + seasonal + noise * climate_.randomnessC;
}

float Weather::TemperatureC(int64_t minute) const {
    const int64_t day = calendar_.DayIndex(minute);
    const float hour = static_cast<float>(minute % kMinutesPerDay) / 60.0f;
    // Warmest around 15:00, coldest around 03:00.
    const float daily = climate_.dailySwingC * std::cos(kTwoPi * (hour - 15.0f) / 24.0f);
    return DailyMeanC(day) + daily;
}

bool Weather::Snowfall(int64_t dayIndex) const {
    return DailyMeanC(dayIndex) < 0.5f && Noise(dayIndex, 1) > -0.1f;
}

float Weather::SnowCover(int64_t dayIndex) const {
    // Replay the last few weeks of weather from bare ground.
    float snow = 0.0f;
    for (int64_t d = dayIndex - kSnowLookbackDays; d <= dayIndex; ++d) {
        const float mean = DailyMeanC(d);
        if (Snowfall(d)) snow += 0.12f + std::max(0.0f, -mean) * 0.01f;
        if (mean > 0.0f) snow -= mean * 0.045f;
        snow = std::clamp(snow, 0.0f, 1.0f);
    }
    return snow;
}

Daylight Weather::DaylightOn(int64_t dayIndex) const {
    const float season = static_cast<float>(calendar_.DaysPerSeason());
    const float midsummer = season * 1.5f;
    const float length = 12.0f + climate_.daylightSwingHours * YearPhase(dayIndex, midsummer);
    const float noon = 12.5f;
    return {noon - length * 0.5f, noon + length * 0.5f};
}

float Weather::Light(int64_t minute) const {
    const Daylight daylight = DaylightOn(calendar_.DayIndex(minute));
    const float hour = static_cast<float>(minute % kMinutesPerDay) / 60.0f;
    const float twilight = 1.0f;
    const float afterSunrise = (hour - daylight.sunrise) / twilight + 0.5f;
    const float beforeSunset = (daylight.sunset - hour) / twilight + 0.5f;
    return std::clamp(std::min(afterSunrise, beforeSunset), 0.0f, 1.0f);
}

}  // namespace tls
