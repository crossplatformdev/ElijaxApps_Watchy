#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawCalories(uint32_t steps) {
  float calories = steps * 0.04f;
  char value[12];
  char stepValue[12];
  snprintf(value, sizeof(value), "%.0f kcal", calories);
  snprintf(stepValue, sizeof(stepValue), "%lu",
           static_cast<unsigned long>(steps));
  AppVisual::drawMetric({12, 39, 176, 90}, "STEP CALORIES", value,
                        static_cast<float>(steps) / dailyStepGoal,
                        "Estimate: 0.04 kcal per step");
  AppVisual::drawDataRow(165, "STEPS", stepValue, true);
}

void draw() { drawCalories(WatchySensor::stepCount()); }

} // namespace

void runStepCalories() {
  runStaticTool("STEP CALORIES", draw);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStepCaloriesPreview(uint8_t) { drawCalories(6842); }
#endif

} // namespace WatchySensorTools
