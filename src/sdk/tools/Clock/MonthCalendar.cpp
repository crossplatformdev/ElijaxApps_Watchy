#include "WatchyUi.h"

#include "AppDisplay.h"
#include "ClockSupport.h"
#include "ClockToolApps.h"

namespace WatchyClockTools {

void drawMonthCalendar(const tmElements_t &time) {
  constexpr int16_t columnLeft = 2;
  constexpr int16_t columnWidth = 28;
  constexpr int16_t weekdayTop = 45;
  constexpr int16_t firstWeekTop = 64;
  constexpr int16_t weekHeight = 21;
  const char *const weekdays[] = {"MO", "TU", "WE", "TH", "FR", "SA", "SU"};
  const uint16_t foreground = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  int year = tmYearToCalendar(time.Year);
  int first = ClockSupport::mondayWeekday(year, time.Month, 1) - 1;
  int count = ClockSupport::daysInMonth(year, time.Month);
  char monthAndYear[16];
  snprintf(monthAndYear, sizeof(monthAndYear), "%s %d",
           monthShortStr(time.Month), year);
  WatchyUi::Canvas::centeredText(WatchyUi::Bounds{0, 25, 200, 17},
                                 monthAndYear, 1, foreground);
  for (uint8_t column = 0; column < 7; column++) {
    WatchyUi::Canvas::centeredText(
        WatchyUi::Bounds{
            static_cast<int16_t>(columnLeft + column * columnWidth),
            weekdayTop, columnWidth, 14},
        weekdays[column], 1, foreground);
  }
  for (int day = 1; day <= count; day++) {
    int cell = first + day - 1;
    int16_t x = columnLeft + (cell % 7) * columnWidth;
    int16_t y = firstWeekTop + (cell / 7) * weekHeight;
    char dayText[4];
    snprintf(dayText, sizeof(dayText), "%d", day);
    WatchyUi::Canvas::centeredText(
        WatchyUi::Bounds{x, y, columnWidth, 16}, dayText, 1, foreground);
    if (day == time.Day) {
      WatchyUi::GrayPaint::fillRoundRect(
          {static_cast<int16_t>(x + 3), static_cast<int16_t>(y - 1), 22, 17},
          3, WatchyUi::Theme::tone(WatchyUi::ToneRole::Selection));
      WatchyUi::Canvas::outline(
          WatchyUi::Bounds{static_cast<int16_t>(x + 4), y, 20, 16},
          foreground);
      WatchyUi::Canvas::centeredText(
          WatchyUi::Bounds{x, y, columnWidth, 16}, dayText, 1, foreground);
    }
  }
}

} // namespace WatchyClockTools
