#include "WatchyUi.h"
#include "Watchy.h"

#include "ClockToolApps.h"


namespace WatchyClockTools {

void drawBinaryClock(const tmElements_t &time) {
  const uint8_t values[] = {time.Hour, time.Minute, time.Second};
  const char *const labels[] = {"HOUR", "MIN", "SEC"};
  const uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  for (uint8_t row = 0; row < 3; row++) {
    int16_t y = 51 + row * 42;
    Watchy::display.setCursor(10, y);
    Watchy::display.print(labels[row]);
    for (uint8_t bit = 0; bit < 6; bit++) {
      int16_t x = 75 + bit * 17;
      bool set = values[row] & (1U << (5 - bit));
      Watchy::display.drawCircle(x, y - 4, 5, foreground);
      if (set) {
        Watchy::display.fillCircle(x, y - 4, 3, foreground);
      }
    }
    Watchy::display.setCursor(176, y);
    Watchy::display.print(values[row]);
  }
  WatchyUi::Widget::footer("BINARY TIME / HOUR MINUTE SECOND");
}

} // namespace WatchyClockTools
