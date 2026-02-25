#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include <math.h>

#include "AppDisplay.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawShakeCount(uint16_t count) {
  beginAppDisplay("SHAKE COUNTER");
  char value[8];
  snprintf(value, sizeof(value), "%u", count);
  AppVisual::drawMetric({12, 42, 176, 88}, "SHAKES", value, -1.0f,
                        "Movement events in this session");
  AppVisual::drawStatusIcon({79, 139, 42, 42}, AppVisual::StatusIcon::SENSOR,
                            true);
  WatchyUi::Widget::footer("SHAKE WATCH  BACK EXIT");
  finishAppDisplay();
}

} // namespace

void runShakeCounter() {
  if (!acquireLiveSensor("SHAKE COUNTER")) return;
  WatchyUi::Input::begin();
  Accel previous{};
  Accel current{};
  readAcceleration(previous);
  uint16_t count = 0;
  uint32_t lastShake = 0;
  drawShakeCount(count);
  while (true) {
    if (WatchyUi::Input::wait(25) == WatchyUi::Event::BACK) break;
    if (readAcceleration(current)) {
      float dx = current.x - previous.x;
      float dy = current.y - previous.y;
      float dz = current.z - previous.z;
      float delta = sqrtf(dx * dx + dy * dy + dz * dz) / 1024.0f;
      if (delta > 1.15f && millis() - lastShake > 350) {
        lastShake = millis();
        count++;
        Watchy::vibMotor(30, 1);
        drawShakeCount(count);
      }
      previous = current;
    }
  }
  releaseLiveSensor();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderShakeCounterPreview(uint8_t view) { drawShakeCount(view == 0 ? 0 : 7); }
#endif

} // namespace WatchySensorTools
