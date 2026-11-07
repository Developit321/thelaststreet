#pragma once

#include <cstdint>

namespace tls {

enum class Season { Spring, Summer, Autumn, Winter };

constexpr int kSeasonCount = 4;
constexpr int kMinutesPerDay = 24 * 60;

const char* SeasonName(Season season);

struct CalendarConfig {
    int startYear = 1900;
    int daysPerSeason = 15;
    int startHour = 6;  // the game begins on day 1 of spring at this hour
};

struct Date {
    int year = 0;
    Season season = Season::Spring;
    int day = 1;          // 1-based day within the season
    int minuteOfDay = 0;  // 0..1439
    int Hour() const { return minuteOfDay / 60; }
    int Minute() const { return minuteOfDay % 60; }
};

// Converts between the game's single running minute count and calendar dates.
// Minute 0 is midnight at the start of spring in the start year.
class Calendar {
public:
    explicit Calendar(const CalendarConfig& config = {});

    int DaysPerSeason() const { return config_.daysPerSeason; }
    int DaysPerYear() const { return config_.daysPerSeason * kSeasonCount; }
    int64_t StartMinute() const { return int64_t{config_.startHour} * 60; }

    Date ToDate(int64_t minute) const;
    int64_t DayIndex(int64_t minute) const { return minute / kMinutesPerDay; }
    // Works for days before the game started too (negative indices).
    int DayOfYear(int64_t dayIndex) const {
        const int64_t perYear = DaysPerYear();
        return static_cast<int>(((dayIndex % perYear) + perYear) % perYear);
    }
    Season SeasonOfDay(int64_t dayIndex) const;

    // How far through the current season, from 0 at its first morning to 1 at its end.
    float SeasonProgress(int64_t minute) const;

    // The first minute of a season (midnight on day 1) in the given year.
    int64_t SeasonStart(int year, Season season) const;

private:
    CalendarConfig config_;
};

}  // namespace tls
