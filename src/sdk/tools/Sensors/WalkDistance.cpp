#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawDistance(uint32_t steps) {
  float kilometers = steps * 0.00075f;
  char value[12];
  char miles[12];
  snprintf(value, sizeof(value), "%.2f km", kilometers);
  snprintf(miles, sizeof(miles), "%.2f mi", kilometers * 0.621371f);
  AppVisual::drawMetric({12, 39, 176, 90}, "WALK DISTANCE", value,
                        static_cast<float>(steps) / dailyStepGoal,
                        "Assumes a 0.75 m stride");
  AppVisual::drawDataRow(165, "MILES", miles, true);
}

void draw() { drawDistance(WatchySensor::stepCount()); }

} // namespace

void runWalkDistance() {
  runStaticTool("WALK DISTANCE", draw);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderWalkDistancePreview(uint8_t) { drawDistance(6842); }
#endif

} // namespace WatchySensorTools
