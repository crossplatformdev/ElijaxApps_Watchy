#include "SafetyToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "SensorManager.h"

namespace WatchySafetyTools {
namespace {

const char *positionName(uint8_t direction) {
  const char *name = "UNKNOWN";
  switch (direction) {
  case DIRECTION_DISP_DOWN: name = "FACE DOWN"; break;
  case DIRECTION_DISP_UP: name = "FACE UP"; break;
  case DIRECTION_BOTTOM_EDGE: name = "BOTTOM EDGE"; break;
  case DIRECTION_TOP_EDGE: name = "TOP EDGE"; break;
  case DIRECTION_RIGHT_EDGE: name = "RIGHT EDGE"; break;
  case DIRECTION_LEFT_EDGE: name = "LEFT EDGE"; break;
  default: break;
  }
  return name;
}

void drawBodyDiagram(uint8_t direction) {
  constexpr WatchyUi::Bounds body{72, 38, 56, 68};
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::GrayPaint::fillRoundRect(
      body, 8, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.drawRoundRect(body.x, body.y, body.width, body.height, 8,
                                foreground);
  int16_t centerX = body.x + body.width / 2;
  int16_t centerY = body.y + body.height / 2;
  if (direction == DIRECTION_DISP_DOWN) {
    Watchy::display.drawLine(centerX, centerY - 16, centerX, centerY + 15,
                             foreground);
    Watchy::display.drawLine(centerX, centerY + 15, centerX - 7, centerY + 8,
                             foreground);
    Watchy::display.drawLine(centerX, centerY + 15, centerX + 7, centerY + 8,
                             foreground);
  } else if (direction == DIRECTION_RIGHT_EDGE ||
             direction == DIRECTION_LEFT_EDGE) {
    int8_t sign = direction == DIRECTION_RIGHT_EDGE ? 1 : -1;
    Watchy::display.drawLine(centerX - 16 * sign, centerY,
                             centerX + 15 * sign, centerY, foreground);
    Watchy::display.drawLine(centerX + 15 * sign, centerY,
                             centerX + 8 * sign, centerY - 7, foreground);
    Watchy::display.drawLine(centerX + 15 * sign, centerY,
                             centerX + 8 * sign, centerY + 7, foreground);
  } else {
    Watchy::display.drawLine(centerX, centerY + 16, centerX, centerY - 15,
                             foreground);
    Watchy::display.drawLine(centerX, centerY - 15, centerX - 7, centerY - 8,
                             foreground);
    Watchy::display.drawLine(centerX, centerY - 15, centerX + 7, centerY - 8,
                             foreground);
  }
}

void drawPosition(uint8_t direction, uint32_t faceDownSeconds, bool warning) {
  beginAppDisplay("BODY POSITION");
  drawBodyDiagram(direction);
  WatchyUi::Canvas::centeredText({0, 115, 200, 19}, positionName(direction), 2,
                                 WatchyUi::Theme::foreground());
  char seconds[12];
  snprintf(seconds, sizeof(seconds), "%lu s",
           static_cast<unsigned long>(faceDownSeconds));
  AppVisual::drawDataRow(153, "FACE-DOWN", seconds, warning);
  AppVisual::drawDataRow(177, "STATUS", warning ? "CHECK NOW" : "Monitoring");
  WatchyUi::Widget::footer("ACTIVE POSTURE MONITOR  BACK STOP");
  finishAppDisplay();
}

} // namespace

void runBodyPosition() {
  WatchyUi::Input::begin();
  if (!WatchySensor::acquireForeground(WatchySensor::Mode::LiveAcceleration)) {
    WatchyUi::Feedback::showMessage(
        "BODY POSITION", "BMA setup failed. Background mode preserved.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    return;
  }
  uint8_t previousDirection = UINT8_MAX;
  uint32_t faceDownStarted = 0;
  uint32_t displayedBucket = UINT32_MAX;
  bool warned = false;
  bool displayedWarning = false;
  while (true) {
    if (WatchyUi::Input::wait(100) == WatchyUi::Event::BACK) break;
    uint8_t direction = WatchySensor::readDirection();
    uint32_t now = millis();
    if (direction == DIRECTION_DISP_DOWN) {
      if (faceDownStarted == 0) faceDownStarted = now;
    } else {
      faceDownStarted = 0;
      warned = false;
    }
    uint32_t seconds = faceDownStarted == 0 ? 0 : (now - faceDownStarted) / 1000;
    if (!warned && seconds >= 30) {
      warned = true;
      Watchy::vibMotor(100, 8);
    }
    uint32_t bucket = seconds / 5;
    if (direction != previousDirection || bucket != displayedBucket ||
        warned != displayedWarning) {
      previousDirection = direction;
      displayedBucket = bucket;
      displayedWarning = warned;
      drawPosition(direction, seconds, warned);
    }
  }
  WatchySensor::releaseForeground(WatchySensor::Mode::LiveAcceleration);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBodyPositionPreview(uint8_t view) {
  drawPosition(view == 0 ? DIRECTION_DISP_UP
                         : view == 1 ? DIRECTION_DISP_DOWN : DIRECTION_RIGHT_EDGE,
               view == 1 ? 34 : 0, view == 1);
}
#endif

} // namespace WatchySafetyTools
