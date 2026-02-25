#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include <Watchy.h>
#include "WatchyUi.h"

extern RTC_DATA_ATTR long gmtOffset;

inline void beginAppDisplay(const char *title) {
  WatchyUi::Screen::begin(title);
}

inline void finishAppDisplay() {
  WatchyUi::Screen::present();
}

inline void beginCompactAppDisplay(const char *title) {
  beginAppDisplay(title);
  WatchyUi::Widget::separator();
  WatchyUi::Widget::bodyText(3, 32);
}

inline void setCompactLine(uint8_t row, uint8_t lineHeight = 17) {
  Watchy::display.setCursor(3, 32 + row * lineHeight);
}

inline void printTwoDigits(uint8_t value) {
  if (value < 10) {
    Watchy::display.print('0');
  }
  Watchy::display.print(value);
}

inline void printDateTime(time_t utcTime, long offset = 0) {
  tmElements_t value;
  breakTime(utcTime + offset, value);
  printTwoDigits(value.Day);
  Watchy::display.print('/');
  printTwoDigits(value.Month);
  Watchy::display.print(' ');
  printTwoDigits(value.Hour);
  Watchy::display.print(':');
  printTwoDigits(value.Minute);
}

inline void printIsoDateTime(time_t utcTime, long offset = 0) {
  tmElements_t value;
  breakTime(utcTime + offset, value);
  Watchy::display.print(tmYearToCalendar(value.Year));
  Watchy::display.print('-');
  printTwoDigits(value.Month);
  Watchy::display.print('-');
  printTwoDigits(value.Day);
  Watchy::display.print(' ');
  printTwoDigits(value.Hour);
  Watchy::display.print(':');
  printTwoDigits(value.Minute);
  Watchy::display.print(':');
  printTwoDigits(value.Second);
}

inline time_t currentUtcTime(tmElements_t currentTime) {
  return makeTime(currentTime) - gmtOffset;
}

namespace AppVisual {

enum class StatusIcon : uint8_t {
  INFO,
  SUCCESS,
  WARNING,
  ERROR,
  LOADING,
  EMPTY,
  RADIO,
  SENSOR,
  TIME
};

void drawStatusIcon(const WatchyUi::Bounds &bounds, StatusIcon icon,
                    bool emphasized = false);
void drawMetric(const WatchyUi::Bounds &bounds, const char *label,
                const char *value, float progress = -1.0f,
                const char *detail = nullptr);
void drawProgressTrack(const WatchyUi::Bounds &bounds, float progress,
                       bool reverse = false, bool marker = false);
void drawSignalBars(const WatchyUi::Bounds &bounds, uint8_t strength,
                    uint8_t maximum = 4, bool framed = false);
void drawMiniChart(const WatchyUi::Bounds &bounds, const int16_t *values,
                   uint8_t count, int16_t minimum, int16_t maximum,
                   bool fill = false);
void drawTimeline(const WatchyUi::Bounds &bounds, float start, float end,
                  float marker = -1.0f);
void drawDataRow(int16_t y, const char *label, const char *value,
                 bool emphasized = false);
// Returns the cursor Y to pass to setCursor() so that a single line of text
// (using the currently active font/size) is vertically centered on centerY.
int16_t centeredCursorY(int16_t centerY, const char *text);
void drawEmptyState(const WatchyUi::Bounds &bounds, const char *label,
                    const char *detail = nullptr);
void drawWarningState(const WatchyUi::Bounds &bounds, const char *label,
                      const char *detail = nullptr);

} // namespace AppVisual

#endif