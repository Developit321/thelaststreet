#include "tls/calendar.h"

namespace tls {

const char* SeasonName(Season season) {
    switch (season) {
        case Season::Spring: return "Spring";
        case Season::Summer: return "Summer";
        case Season::Autumn: return "Autumn";
        case Season::Winter: return "Winter";
    }
    return "?";
}

Calendar::Calendar(const CalendarConfig& config) : config_(config) {
    if (config_.daysPerSeason < 1) config_.daysPerSeason = 1;
    if (config_.startHour < 0 || config_.startHour > 23) config_.startHour = 6;
}

Season Calendar::SeasonOfDay(int64_t dayIndex) const {
    return static_cast<Season>(DayOfYear(dayIndex) / config_.daysPerSeason);
}

Date Calendar::ToDate(int64_t minute) const {
    const int64_t dayIndex = DayIndex(minute);
    const int dayOfYear = DayOfYear(dayIndex);
    Date date;
    date.year = config_.startYear + static_cast<int>(dayIndex / DaysPerYear());
    date.season = static_cast<Season>(dayOfYear / config_.daysPerSeason);
    date.day = dayOfYear % config_.daysPerSeason + 1;
    date.minuteOfDay = static_cast<int>(minute % kMinutesPerDay);
    return date;
}

float Calendar::SeasonProgress(int64_t minute) const {
    const Date date = ToDate(minute);
    const float days = static_cast<float>(date.day - 1) +
                       static_cast<float>(date.minuteOfDay) / kMinutesPerDay;
    return days / static_cast<float>(config_.daysPerSeason);
}

int64_t Calendar::SeasonStart(int year, Season season) const {
    const int64_t years = year - config_.startYear;
    const int64_t day = years * DaysPerYear() + static_cast<int>(season) * config_.daysPerSeason;
    return day * kMinutesPerDay;
}

}  // namespace tls
