#include "WatchyUi.h"
#include "AppDisplay.h"
#include "ClockToolApps.h"
#include "Watchy.h"

namespace {

enum ClockTool : uint8_t {
  BINARY_CLOCK,
  UNIX_TIME,
  UTC_CLOCK,
  WEEK_NUMBER,
  DAY_OF_YEAR,
  MONTH_CALENDAR,
  WORLD_CLOCKS,
  DUAL_TIME,
  INTERNET_BEATS,
  DECIMAL_TIME,
  JULIAN_DAY,
  TIME_PROGRESS,
  CLOCK_TOOL_COUNT
};

const char *const titles[CLOCK_TOOL_COUNT] = {
    "BINARY CLOCK", "UNIX TIME",      "UTC CLOCK",     "ISO WEEK",
    "DAY OF YEAR",  "MONTH CALENDAR", "WORLD CLOCKS",  "LOCAL + UTC",
    "INTERNET BEATS", "DECIMAL TIME", "JULIAN DAY",    "TIME PROGRESS"};

void drawClockTool(uint8_t tool, const tmElements_t &time) {
  switch (tool) {
  case BINARY_CLOCK: WatchyClockTools::drawBinaryClock(time); break;
  case UNIX_TIME: WatchyClockTools::drawUnixTime(time); break;
  case UTC_CLOCK: WatchyClockTools::drawUtcClock(time); break;
  case WEEK_NUMBER: WatchyClockTools::drawWeekNumber(time); break;
  case DAY_OF_YEAR: WatchyClockTools::drawDayOfYear(time); break;
  case MONTH_CALENDAR: WatchyClockTools::drawMonthCalendar(time); break;
  case WORLD_CLOCKS: WatchyClockTools::drawWorldClocks(time); break;
  case DUAL_TIME: WatchyClockTools::drawDualTime(time); break;
  case INTERNET_BEATS: WatchyClockTools::drawInternetBeats(time); break;
  case DECIMAL_TIME: WatchyClockTools::drawDecimalTime(time); break;
  case JULIAN_DAY: WatchyClockTools::drawJulianDay(time); break;
  case TIME_PROGRESS: WatchyClockTools::drawTimeProgress(time); break;
  default: break;
  }
}

} // namespace

void showClockToolImpl(uint8_t tool, Watchy *watchy) {
  if (tool >= CLOCK_TOOL_COUNT) {
    tool = BINARY_CLOCK;
  }
  WatchyUi::Input::begin();
  while (true) {
    tmElements_t currentTime;
    if (watchy != nullptr) {
      watchy->RTC.read(watchy->currentTime);
      currentTime = watchy->currentTime;
    } else {
      WatchySdk::RTC.read(WatchySdk::currentTime);
      currentTime = WatchySdk::currentTime;
    }
    beginAppDisplay(titles[tool]);

    drawClockTool(tool, currentTime);

    finishAppDisplay();
    if (WatchyUi::Input::wait(
            WatchyUi::Screen::liveViewRefreshIntervalMs) ==
        WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }
  }
}

void Watchy::showClockTool(uint8_t tool) { showClockToolImpl(tool, this); }

void WatchySdk::showClockTool(uint8_t tool) { showClockToolImpl(tool, nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderClockPreview(uint8_t tool, uint8_t view,
                        const tmElements_t &fixedTime) {
  (void)view;
  if (tool >= CLOCK_TOOL_COUNT) {
    tool = BINARY_CLOCK;
  }

  long previousOffset = gmtOffset;
  gmtOffset = 2 * SECS_PER_HOUR;
  beginAppDisplay(titles[tool]);
  drawClockTool(tool, fixedTime);
  finishAppDisplay();
  gmtOffset = previousOffset;
}

} // namespace WatchyDemo
#endif
