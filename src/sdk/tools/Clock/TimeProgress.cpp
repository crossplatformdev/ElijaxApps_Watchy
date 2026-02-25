#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "ClockSupport.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {
namespace {

void useSmallText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void drawBar(int16_t y, float progress) {
  AppVisual::drawProgressTrack({4, y, 192, 13}, progress);
}

} // namespace

void drawTimeProgress(const tmElements_t &time) {
  int year = tmYearToCalendar(time.Year);
  float minute = time.Second / 60.0f;
  float day = (time.Hour * 3600.0f + time.Minute * 60.0f + time.Second) /
              SECS_PER_DAY;
  float yearProgress =
      (ClockSupport::ordinalDay(year, time.Month, time.Day) - 1 + day) /
      (ClockSupport::isLeapYear(year) ? 366.0f : 365.0f);
  useSmallText(8, 41);
  Watchy::display.print("MINUTE");
  drawBar(48, minute);
  Watchy::display.setCursor(8, 84);
  Watchy::display.print("DAY");
  drawBar(91, day);
  Watchy::display.setCursor(8, 127);
  Watchy::display.print("YEAR");
  drawBar(134, yearProgress);
  char value[12];
  snprintf(value, sizeof(value), "%.1f%%", yearProgress * 100.0f);
  AppVisual::drawDataRow(177, "YEAR COMPLETE", value, true);
}

} // namespace WatchyClockTools
