#include "SensorToolApps.h"

#include "WatchyUi.h"

#include <math.h>

#include "AppDisplay.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

float magnitude(const Accel &acceleration) {
  float x = acceleration.x / 1024.0f;
  float y = acceleration.y / 1024.0f;
  float z = acceleration.z / 1024.0f;
  return sqrtf(x * x + y * y + z * z);
}

void drawGForce(const Accel &acceleration) {
  float x = acceleration.x / 1024.0f;
  float y = acceleration.y / 1024.0f;
  float z = acceleration.z / 1024.0f;
  char total[12];
  snprintf(total, sizeof(total), "%.2f g", magnitude(acceleration));
  AppVisual::drawMetric({12, 32, 176, 57}, "TOTAL ACCELERATION", total);
  drawAxisValue("X", x, 2.0f, 116);
  drawAxisValue("Y", y, 2.0f, 140);
  drawAxisValue("Z", z, 2.0f, 164);
}

void draw(const Accel &acceleration) { drawGForce(acceleration); }

} // namespace

void runGForce() { runLiveTool("G FORCE", true, draw); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderGForcePreview(uint8_t) {
  drawGForce(Accel{184, -92, 1002});
}
#endif

} // namespace WatchySensorTools
