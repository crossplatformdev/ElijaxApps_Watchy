#include "UtilitySupport.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"

namespace WatchyUtilityTools {

void runGenerator(Generator generator, GeneratorRenderer renderer) {
  WatchyUi::Input::begin();
  char output[40];
  while (true) {
    generator(output, sizeof(output));
    renderer(output);
    while (true) {
      WatchyUi::Event event = WatchyUi::Input::wait();
      if (event == WatchyUi::Event::BACK) return;
      if (event == WatchyUi::Event::MENU) break;
    }
  }
}

void runAdjustableValue(int initialValue, int step, int minimum, int maximum,
                        ValueRenderer renderer) {
  WatchyUi::Input::begin();
  int value = initialValue;
  renderer(value);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      value = min(maximum, value + step);
      renderer(value);
    } else if (event == WatchyUi::Event::DOWN) {
      value = max(minimum, value - step);
      renderer(value);
    }
  }
}

void useSmallText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void drawCoinFace(const char *result) {
  constexpr int16_t centerX = 100;
  constexpr int16_t centerY = 89;
  constexpr int16_t radius = 43;
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::GrayPaint::fillRoundRect(
      {52, 41, 96, 96}, 48,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.drawCircle(centerX, centerY, radius, foreground);
  Watchy::display.drawCircle(centerX, centerY, radius - 6, foreground);
  WatchyUi::Canvas::centeredText({66, 55, 68, 68},
                                 result[0] == 'H' ? "H" : "T", 5,
                                 foreground);
  WatchyUi::Canvas::centeredText({0, 142, 200, 18}, result, 2, foreground);
}

void drawDieFace(uint8_t sides, uint8_t value) {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  if (sides == 6) {
    constexpr int16_t left = 55;
    constexpr int16_t top = 38;
    constexpr int16_t size = 90;
    WatchyUi::GrayPaint::fillRoundRect(
        {left, top, size, size}, 8,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    Watchy::display.drawRoundRect(left, top, size, size, 8, foreground);
    const int16_t low = left + 23;
    const int16_t middle = left + size / 2;
    const int16_t high = left + size - 23;
    const int16_t upper = top + 23;
    const int16_t center = top + size / 2;
    const int16_t lower = top + size - 23;
    if (value == 1 || value == 3 || value == 5)
      Watchy::display.fillCircle(middle, center, 5, foreground);
    if (value >= 2) {
      Watchy::display.fillCircle(low, upper, 5, foreground);
      Watchy::display.fillCircle(high, lower, 5, foreground);
    }
    if (value >= 4) {
      Watchy::display.fillCircle(high, upper, 5, foreground);
      Watchy::display.fillCircle(low, lower, 5, foreground);
    }
    if (value == 6) {
      Watchy::display.fillCircle(low, center, 5, foreground);
      Watchy::display.fillCircle(high, center, 5, foreground);
    }
  } else {
    WatchyUi::GrayPaint::fillRoundRect(
        {46, 38, 108, 96}, 6,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    Watchy::display.drawTriangle(100, 39, 51, 129, 149, 129, foreground);
    WatchyUi::Canvas::centeredText({46, 38, 108, 96}, "D20", 2, foreground);
  }
  char result[5];
  snprintf(result, sizeof(result), "%u", value);
  WatchyUi::Canvas::centeredText({0, 145, 200, 26}, result, 4, foreground);
}

void drawConverterArrow() {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawLine(78, 95, 122, 95, foreground);
  Watchy::display.drawLine(122, 95, 114, 89, foreground);
  Watchy::display.drawLine(122, 95, 114, 101, foreground);
}

} // namespace WatchyUtilityTools
