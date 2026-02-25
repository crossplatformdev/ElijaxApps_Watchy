#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawGoal(uint32_t steps) {
  char value[12];
  char remaining[14];
  snprintf(value, sizeof(value), "%lu", static_cast<unsigned long>(steps));
  snprintf(remaining, sizeof(remaining), "%lu", static_cast<unsigned long>(
      steps >= dailyStepGoal ? 0 : dailyStepGoal - steps));
  AppVisual::drawMetric({12, 39, 176, 90}, "STEP GOAL / 10K", value,
                        static_cast<float>(steps) / dailyStepGoal,
                        steps >= dailyStepGoal ? "Goal reached" : "Today");
  AppVisual::drawDataRow(165, "REMAINING", remaining, true);
}

void draw() { drawGoal(WatchySensor::stepCount()); }

} // namespace

void runStepGoal() { runStaticTool("STEP GOAL", draw); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStepGoalPreview(uint8_t view) { drawGoal(view == 0 ? 6842 : 10537); }
#endif

} // namespace WatchySensorTools
