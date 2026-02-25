#include "SensorToolApps.h"

#include "WatchyUi.h"

#include <math.h>

#include "AppDisplay.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawScore(float score) {
  char value[10];
  const char *state = score < 0.03f ? "STILL"
                                    : score < 0.15f ? "MOVING" : "ACTIVE";
  snprintf(value, sizeof(value), "%.3f", score);
  AppVisual::drawMetric({12, 39, 176, 89}, "MOTION SCORE", value,
                        constrain(score / 0.30f, 0.0f, 1.0f),
                        "Mean delta-g per sample");
  AppVisual::drawDataRow(164, "ACTIVITY", state, true);
}

void render(const Accel &) {
  Accel previous{};
  Accel current{};
  if (!readAcceleration(previous)) {
    drawSensorReadFailure();
    return;
  }
  float totalDelta = 0.0f;
  for (uint8_t sample = 0; sample < 40; sample++) {
    WatchyUi::deepSleepDelay(20);
    if (readAcceleration(current)) {
      float dx = current.x - previous.x;
      float dy = current.y - previous.y;
      float dz = current.z - previous.z;
      totalDelta += sqrtf(dx * dx + dy * dy + dz * dz) / 1024.0f;
      previous = current;
    }
  }
  drawScore(totalDelta / 40.0f);
}

} // namespace

void runMotionScore() {
  runLiveTool("MOTION SCORE", true, render);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderMotionScorePreview(uint8_t view) {
  const float scores[] = {0.012f, 0.084f, 0.260f};
  drawScore(scores[min<uint8_t>(view, 2)]);
}
#endif

} // namespace WatchySensorTools
