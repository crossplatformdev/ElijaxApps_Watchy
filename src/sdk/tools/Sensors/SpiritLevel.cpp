#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include <math.h>

#include "AppDisplay.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

struct LevelReading {
  float inclinationX;
  float inclinationY;
  int16_t dotX;
  int16_t dotY;
  int16_t displayedX;
  int16_t displayedY;
};

LevelReading levelReading(const Accel &acceleration) {
  float x = acceleration.x / 1024.0f;
  float y = acceleration.y / 1024.0f;
  float z = acceleration.z / 1024.0f;
  float inclinationX = atan2f(x, sqrtf(x * x + z * z)) * 180.0f / M_PI;
  float inclinationY = atan2f(y, sqrtf(y * y + z * z)) * 180.0f / M_PI;
  return {inclinationX, inclinationY,
          static_cast<int16_t>(100 + constrain(static_cast<int>(inclinationX),
                                                -50, 50)),
          static_cast<int16_t>(105 + constrain(static_cast<int>(inclinationY),
                                                -50, 50)),
          static_cast<int16_t>(lroundf(inclinationX)),
          static_cast<int16_t>(lroundf(inclinationY))};
}

void drawFrame() {
  uint16_t color = WatchyUi::Theme::foreground();
  Watchy::display.drawCircle(100, 105, 58, color);
  Watchy::display.drawLine(42, 105, 158, 105, color);
  Watchy::display.drawLine(100, 47, 100, 163, color);
}

void drawDot(const LevelReading &reading) {
  Watchy::display.fillCircle(reading.dotX, reading.dotY, 5,
                            WatchyUi::Theme::foreground());
}

void drawText(const LevelReading &reading) {
  useBodyText(4, 184);
  Watchy::display.print("X ");
  Watchy::display.print(reading.inclinationX, 0);
  Watchy::display.print(" DEG  Y ");
  Watchy::display.print(reading.inclinationY, 0);
  Watchy::display.print(" DEG");
}

WatchyUi::Bounds dotBounds(const LevelReading &reading) {
  return {static_cast<int16_t>(reading.dotX - 6),
          static_cast<int16_t>(reading.dotY - 6), 13, 13};
}

bool same(const LevelReading &first, const LevelReading &second) {
  return first.dotX == second.dotX && first.dotY == second.dotY &&
         first.displayedX == second.displayedX &&
         first.displayedY == second.displayedY;
}

} // namespace

void runSpiritLevel() {
  if (!acquireLiveSensor("SPIRIT LEVEL")) return;
  WatchyUi::Input::begin();
  Accel acceleration{};
  bool valid = readAcceleration(acceleration);
  LevelReading visible = valid ? levelReading(acceleration) : LevelReading{};
  beginAppDisplay("SPIRIT LEVEL");
  if (valid) {
    drawFrame();
    drawDot(visible);
    drawText(visible);
  } else {
    drawSensorReadFailure();
  }
  finishAppDisplay();

  while (true) {
    if (WatchyUi::Input::wait(WatchyUi::Screen::liveViewRefreshIntervalMs) ==
        WatchyUi::Event::BACK) {
      break;
    }
    bool currentValid = readAcceleration(acceleration);
    if (currentValid != valid) {
      valid = currentValid;
      visible = valid ? levelReading(acceleration) : LevelReading{};
      beginAppDisplay("SPIRIT LEVEL");
      if (valid) {
        drawFrame();
        drawDot(visible);
        drawText(visible);
      } else {
        drawSensorReadFailure();
      }
      finishAppDisplay();
      continue;
    }
    if (!valid) continue;

    LevelReading current = levelReading(acceleration);
    if (same(visible, current)) continue;
    if (visible.dotX != current.dotX || visible.dotY != current.dotY) {
      WatchyUi::Bounds oldDot = dotBounds(visible);
      WatchyUi::Bounds newDot = dotBounds(current);
      Watchy::display.fillRect(oldDot.x, oldDot.y, oldDot.width, oldDot.height,
                               WatchyUi::Theme::background());
      Watchy::display.fillRect(newDot.x, newDot.y, newDot.width, newDot.height,
                               WatchyUi::Theme::background());
      drawFrame();
      drawDot(current);
      WatchyUi::Screen::invalidate(oldDot);
      WatchyUi::Screen::invalidate(newDot);
    }
    if (visible.displayedX != current.displayedX ||
        visible.displayedY != current.displayedY) {
      constexpr WatchyUi::Bounds textBounds{0, 170, 200, 22};
      Watchy::display.fillRect(textBounds.x, textBounds.y, textBounds.width,
                               textBounds.height, WatchyUi::Theme::background());
      drawText(current);
      WatchyUi::Screen::invalidate(textBounds);
    }
    WatchyUi::Screen::presentDirty(APP_STATE);
    visible = current;
  }
  releaseLiveSensor();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderSpiritLevelPreview(uint8_t view) {
  Accel acceleration = view == 0 ? Accel{0, 0, 1024} : Accel{184, -92, 1002};
  LevelReading reading = levelReading(acceleration);
  drawFrame();
  drawDot(reading);
  drawText(reading);
}
#endif

} // namespace WatchySensorTools
