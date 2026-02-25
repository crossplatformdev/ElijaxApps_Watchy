#ifndef WATCHY_CLOCK_TOOL_APPS_H
#define WATCHY_CLOCK_TOOL_APPS_H

#include <Arduino.h>
#include <TimeLib.h>

namespace WatchyClockTools {

void drawBinaryClock(const tmElements_t &time);
void drawUnixTime(const tmElements_t &time);
void drawUtcClock(const tmElements_t &time);
void drawWeekNumber(const tmElements_t &time);
void drawDayOfYear(const tmElements_t &time);
void drawMonthCalendar(const tmElements_t &time);
void drawWorldClocks(const tmElements_t &time);
void drawDualTime(const tmElements_t &time);
void drawInternetBeats(const tmElements_t &time);
void drawDecimalTime(const tmElements_t &time);
void drawJulianDay(const tmElements_t &time);
void drawTimeProgress(const tmElements_t &time);

} // namespace WatchyClockTools

#endif