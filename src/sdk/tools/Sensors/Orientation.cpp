#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

const char *name(uint8_t direction) {
  const char *label = "UNKNOWN";
  switch (direction) {
  case DIRECTION_DISP_DOWN: label = "FACE DOWN"; break;
  case DIRECTION_DISP_UP: label = "FACE UP"; break;
  case DIRECTION_BOTTOM_EDGE: label = "BOTTOM EDGE"; break;
  case DIRECTION_TOP_EDGE: label = "TOP EDGE"; break;
  case DIRECTION_RIGHT_EDGE: label = "RIGHT EDGE"; break;
  case DIRECTION_LEFT_EDGE: label = "LEFT EDGE"; break;
  default: break;
  }
  return label;
}

void drawDiagram(const char *direction) {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  constexpr WatchyUi::Bounds watchBounds{68, 42, 64, 75};
  WatchyUi::GrayPaint::fillRoundRect(
      watchBounds, 8, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.drawRoundRect(watchBounds.x, watchBounds.y, watchBounds.width,
                                watchBounds.height, 8, foreground);
  int16_t centerX = watchBounds.x + watchBounds.width / 2;
  int16_t centerY = watchBounds.y + watchBounds.height / 2;
  if (strstr(direction, "DOWN") != nullptr) {
    Watchy::display.drawLine(centerX, centerY - 18, centerX, centerY + 17,
                             foreground);
    Watchy::display.drawLine(centerX, centerY + 17, centerX - 7, centerY + 9,
                             foreground);
    Watchy::display.drawLine(centerX, centerY + 17, centerX + 7, centerY + 9,
                             foreground);
  } else if (strstr(direction, "RIGHT") != nullptr) {
    Watchy::display.drawLine(centerX - 18, centerY, centerX + 17, centerY,
                             foreground);
    Watchy::display.drawLine(centerX + 17, centerY, centerX + 9, centerY - 7,
                             foreground);
    Watchy::display.drawLine(centerX + 17, centerY, centerX + 9, centerY + 7,
                             foreground);
  } else {
    Watchy::display.drawLine(centerX, centerY + 18, centerX, centerY - 17,
                             foreground);
    Watchy::display.drawLine(centerX, centerY - 17, centerX - 7, centerY - 9,
                             foreground);
    Watchy::display.drawLine(centerX, centerY - 17, centerX + 7, centerY - 9,
                             foreground);
  }
}

void draw(const char *direction) {
  drawDiagram(direction);
  WatchyUi::Canvas::centeredText({0, 128, 200, 21}, direction, 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(172, "DOMINANT AXIS", "BMA423", true);
}

void render(const Accel &) { draw(name(WatchySensor::readDirection())); }

} // namespace

void runOrientation() {
  runLiveTool("ORIENTATION", true, render);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderOrientationPreview(uint8_t view) {
  const char *const orientations[] = {"FACE UP", "FACE DOWN", "RIGHT EDGE"};
  draw(orientations[min<uint8_t>(view, 2)]);
}
#endif

} // namespace WatchySensorTools
