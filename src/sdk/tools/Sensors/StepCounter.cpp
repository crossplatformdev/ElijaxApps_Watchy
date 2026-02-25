#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawSteps(uint32_t steps) {
  char value[12];
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(steps));
  AppVisual::drawMetric({12, 39, 176, 90}, "STEP COUNTER", value,
                        static_cast<float>(steps) / dailyStepGoal,
                        "Today against 10,000 steps");
  AppVisual::drawDataRow(165, "SENSOR", "BMA423 podometer", true);
}

void draw() { drawSteps(WatchySensor::stepCount()); }

} // namespace

void runStepCounter() { runStaticTool("STEP COUNTER", draw); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStepCounterPreview(uint8_t) { drawSteps(6842); }
#endif

} // namespace WatchySensorTools
