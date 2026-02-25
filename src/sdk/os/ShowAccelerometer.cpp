#include "WatchyUi.h"
#include "AppDisplay.h"
#include "SensorManager.h"
#include "Watchy.h"

namespace {

constexpr WatchyUi::Bounds valuesBounds{8, 40, 184, 70};
constexpr WatchyUi::Bounds directionBounds{12, 122, 176, 22};

struct VisibleAcceleration {
  int16_t x;
  int16_t y;
  int16_t z;
  uint8_t direction;
  bool valid;
};

bool valuesChanged(const VisibleAcceleration &previous,
                   const VisibleAcceleration &current) {
  return previous.valid != current.valid || previous.x != current.x ||
         previous.y != current.y || previous.z != current.z;
}

void drawValues(const VisibleAcceleration &value) {
  WatchyUi::GrayPaint::fillRoundRect(
      valuesBounds, 4, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  if (!value.valid) {
    AppVisual::drawWarningState(valuesBounds.inset(4), "SENSOR READ FAILED",
                                "Try again or check BMA setup");
    return;
  }
  const char *const labels[] = {"X", "Y", "Z"};
  const int16_t readings[] = {value.x, value.y, value.z};
  const uint16_t foreground = WatchyUi::Theme::foreground();
  for (uint8_t row = 0; row < 3; row++) {
    int16_t y = valuesBounds.y + 17 + row * 21;
    int16_t center = 91;
    int16_t extent = constrain(readings[row], -1024, 1024) * 45 / 1024;
    Watchy::display.setFont();
    Watchy::display.setTextSize(1);
    Watchy::display.setCursor(16, AppVisual::centeredCursorY(y, labels[row]));
    Watchy::display.print(labels[row]);
    WatchyUi::GrayPaint::line(42, y, 140, y,
                               WatchyUi::Theme::tone(WatchyUi::ToneRole::Separator));
    Watchy::display.drawLine(center, y - 5, center, y + 5, foreground);
    Watchy::display.fillRect(extent >= 0 ? center : center + extent, y - 3,
                              max<int16_t>(1, abs(extent)), 7, foreground);
    char valueText[8];
    snprintf(valueText, sizeof(valueText), "%d", readings[row]);
    Watchy::display.setCursor(149, AppVisual::centeredCursorY(y, valueText));
    Watchy::display.print(valueText);
  }
}

const char *directionName(uint8_t direction) {
  switch (direction) {
  case DIRECTION_DISP_DOWN: return "FACE DOWN";
  case DIRECTION_DISP_UP: return "FACE UP";
  case DIRECTION_BOTTOM_EDGE: return "BOTTOM EDGE";
  case DIRECTION_TOP_EDGE: return "TOP EDGE";
  case DIRECTION_RIGHT_EDGE: return "RIGHT EDGE";
  case DIRECTION_LEFT_EDGE: return "LEFT EDGE";
  default: return "UNKNOWN";
  }
}

void drawDirection(uint8_t direction) {
  WatchyUi::GrayPaint::fillRoundRect(
      directionBounds, 3,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  WatchyUi::Canvas::centeredText(directionBounds, directionName(direction), 1,
                                 WatchyUi::Theme::foreground());
}

VisibleAcceleration readVisibleAcceleration() {
  Accel acceleration{};
    bool valid = WatchySensor::readAcceleration(acceleration);
  return {acceleration.x, acceleration.y, acceleration.z,
      WatchySensor::readDirection(), valid};
}

void drawAccelerometerFrame() {
  WatchyUi::Screen::begin("ACCELEROMETER");
  AppVisual::drawStatusIcon({79, 22, 42, 16}, AppVisual::StatusIcon::SENSOR);
  WatchyUi::Widget::footer("LIVE AXES  BACK EXIT");
}

} // namespace

void showAccelerometerImpl(Watchy *watchy) {
  if (!WatchySensor::acquireForeground(
          WatchySensor::Mode::LiveAcceleration)) {
    WatchyUi::Feedback::showMessage(
        "ACCELEROMETER", "BMA setup failed. Background mode preserved.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    WatchyUi::Input::begin();
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    if (watchy != nullptr) {
      watchy->showMenu(menuIndex, false);
    } else {
      WatchySdk::showMenu(menuIndex, false);
    }
    return;
  }
  guiState = APP_STATE;

  WatchyUi::Input::begin();
  VisibleAcceleration visible = readVisibleAcceleration();
  drawAccelerometerFrame();
  drawValues(visible);
  drawDirection(visible.direction);
  WatchyUi::Screen::present(APP_STATE);

  while (true) {
    if (WatchyUi::Input::wait(
            WatchyUi::Screen::liveViewRefreshIntervalMs) ==
        WatchyUi::Event::BACK) {
      break;
    }
    VisibleAcceleration current = readVisibleAcceleration();
    if (valuesChanged(visible, current)) {
      drawValues(current);
      WatchyUi::Screen::invalidate(valuesBounds);
    }
    if (visible.direction != current.direction ||
        visible.valid != current.valid) {
      drawDirection(current.direction);
      WatchyUi::Screen::invalidate(directionBounds);
    }
    WatchyUi::Screen::presentDirty(APP_STATE);
    visible = current;
  }

  WatchySensor::releaseForeground(WatchySensor::Mode::LiveAcceleration);
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

void Watchy::showAccelerometer() {
  showAccelerometerImpl(this);
}

void WatchySdk::showAccelerometer() {
  showAccelerometerImpl(nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderAccelerometerPreview(uint8_t view) {
  VisibleAcceleration preview{
      static_cast<int16_t>(view == 0 ? 184 : -42),
      static_cast<int16_t>(view == 0 ? -92 : 65),
      static_cast<int16_t>(view == 0 ? 1002 : -1008),
      static_cast<uint8_t>(view == 0 ? DIRECTION_DISP_UP : DIRECTION_DISP_DOWN),
      view < 2};
  drawAccelerometerFrame();
  drawValues(preview);
  drawDirection(preview.direction);
  WatchyUi::Screen::present(APP_STATE);
}

} // namespace WatchyDemo
#endif
