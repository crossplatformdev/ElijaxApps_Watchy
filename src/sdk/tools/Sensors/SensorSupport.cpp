#include "SensorSupport.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "SensorManager.h"

namespace WatchySensorTools {

void useBodyText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

bool readAcceleration(Accel &acceleration) {
  return WatchySensor::readAcceleration(acceleration);
}

void drawAxisValue(const char *label, float value, float maximum, int16_t y,
                   bool integer) {
  constexpr int16_t barX = 42;
  constexpr int16_t barWidth = 106;
  const uint16_t foreground = WatchyUi::Theme::foreground();
  float ratio = constrain(value / maximum, -1.0f, 1.0f);
  int16_t center = barX + barWidth / 2;
  int16_t extent = static_cast<int16_t>(ratio * (barWidth / 2 - 3));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, AppVisual::centeredCursorY(y, label));
  Watchy::display.print(label);
  WatchyUi::GrayPaint::line(
      barX, y, barX + barWidth, y,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Separator));
  Watchy::display.drawLine(center, y - 5, center, y + 5, foreground);
  if (extent >= 0) {
    Watchy::display.fillRect(center, y - 3, max<int16_t>(1, extent), 7,
                              foreground);
  } else {
    Watchy::display.fillRect(center + extent, y - 3,
                              max<int16_t>(1, -extent), 7, foreground);
  }
  char valueText[12];
  if (integer) {
    snprintf(valueText, sizeof(valueText), "%ld", static_cast<long>(value));
  } else {
    snprintf(valueText, sizeof(valueText), "%.2f", value);
  }
  Watchy::display.setCursor(155, AppVisual::centeredCursorY(y, valueText));
  Watchy::display.print(valueText);
}

void drawSensorReadFailure() {
  useBodyText();
  Watchy::display.println("Sensor read failed");
}

bool acquireLiveSensor(const char *title) {
  if (WatchySensor::acquireForeground(WatchySensor::Mode::LiveAcceleration)) {
    return true;
  }
  WatchyUi::Feedback::showMessage(
      title, "BMA setup failed. Background mode preserved.",
      WatchyUi::MessageKind::ERROR, "BACK EXIT");
  WatchyUi::Input::begin();
  while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
  return false;
}

void releaseLiveSensor() {
  WatchySensor::releaseForeground(WatchySensor::Mode::LiveAcceleration);
}

void runStaticTool(const char *title, StaticRenderer renderer,
                   uint32_t refreshIntervalMs) {
  WatchyUi::Input::begin();
  while (true) {
    beginAppDisplay(title);
    renderer();
    finishAppDisplay();
    if (WatchyUi::Input::wait(refreshIntervalMs) == WatchyUi::Event::BACK) {
      return;
    }
  }
}

void runLiveTool(const char *title, bool requireSample,
                 LiveRenderer renderer) {
  if (!acquireLiveSensor(title)) return;
  WatchyUi::Input::begin();
  while (true) {
    beginAppDisplay(title);
    Accel acceleration{};
    if (requireSample && !readAcceleration(acceleration)) {
      drawSensorReadFailure();
    } else {
      renderer(acceleration);
    }
    finishAppDisplay();
    if (WatchyUi::Input::wait(WatchyUi::Screen::liveViewRefreshIntervalMs) ==
        WatchyUi::Event::BACK) {
      break;
    }
  }
  releaseLiveSensor();
}

} // namespace WatchySensorTools
