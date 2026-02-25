#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawRawAcceleration(const Accel &acceleration) {
  drawAxisValue("X", acceleration.x, 1024.0f, 57, true);
  drawAxisValue("Y", acceleration.y, 1024.0f, 92, true);
  drawAxisValue("Z", acceleration.z, 1024.0f, 127, true);
  AppVisual::drawDataRow(168, "SCALE", "1024 LSB/g", true);
}

void draw(const Accel &acceleration) { drawRawAcceleration(acceleration); }

} // namespace

void runRawAccel() { runLiveTool("RAW ACCEL", true, draw); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderRawAccelPreview(uint8_t) {
  drawRawAcceleration(Accel{184, -92, 1002});
}
#endif

} // namespace WatchySensorTools
