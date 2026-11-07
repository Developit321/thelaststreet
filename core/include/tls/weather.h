#pragma once

#include <cstdint>

#include "tls/calendar.h"

namespace tls {

struct ClimateConfig {
    float meanC = 2.0f;            // average temperature over the year
    float seasonalSwingC = 18.0f;  // how far midsummer and midwinter sit from the average
    float dailySwingC = 5.0f;      // afternoon warmth vs. pre-dawn cold
    float randomnessC = 4.0f;      // day-to-day weather variation
    float daylightSwingHours = 6.0f;  // midsummer days are 12 + this long, midwinter 12 - this
    uint32_t seed = 1900;
};

struct Daylight {
    float sunrise = 6.0f;  // hour of day
    float sunset = 18.0f;
};

// The weather is a pure function of the date: the same day always has the
// same temperature and snow, so jumping around in time stays consistent and
// two runs with the same seed match exactly.
class Weather {
public:
    Weather(const Calendar& calendar, const ClimateConfig& climate);

    float DailyMeanC(int64_t dayIndex) const;
    float TemperatureC(int64_t minute) const;

    // Fresh snowfall happens on some freezing days; snow melts on warm ones.
    bool Snowfall(int64_t dayIndex) const;
    // 0 = bare ground, 1 = deep snow.
    float SnowCover(int64_t dayIndex) const;

    Daylight DaylightOn(int64_t dayIndex) const;
    // 0 at night, 1 in daylight, blending through an hour of twilight.
    float Light(int64_t minute) const;

private:
    float Noise(int64_t dayIndex, uint32_t channel) const;  // -1..1
    float YearPhase(int64_t dayIndex, float peakDay) const;  // cos wave, 1 at peakDay

    Calendar calendar_;
    ClimateConfig climate_;
};

}  // namespace tls
