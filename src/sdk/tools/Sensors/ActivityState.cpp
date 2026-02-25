#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

const char *activityName(const char *activity) {
  if (strstr(activity, "STATIONARY") != nullptr) return "STATIONARY";
  if (strstr(activity, "WALKING") != nullptr) return "WALKING";
  if (strstr(activity, "RUNNING") != nullptr) return "RUNNING";
  return "UNKNOWN";
}

void drawActivity(const char *activity) {
  AppVisual::drawStatusIcon({79, 38, 42, 42}, AppVisual::StatusIcon::SENSOR,
                            true);
  WatchyUi::Canvas::centeredText({0, 94, 200, 24}, activity, 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(158, "CLASSIFIER", "BMA423 motion", true);
}

void draw(const Accel &) {
  bool enabled = WatchySensor::setActivityEnabled(true);
  if (enabled) WatchyUi::deepSleepDelay(1200);
  const char *activity = enabled ? activityName(WatchySensor::readActivity())
                                 : "UNAVAILABLE";
  if (enabled) WatchySensor::setActivityEnabled(false);
  drawActivity(activity);
}

} // namespace

void runActivityState() {
  runLiveTool("ACTIVITY", false, draw);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderActivityStatePreview(uint8_t view) {
  const char *const activities[] = {"STATIONARY", "WALKING", "RUNNING"};
  drawActivity(activities[min<uint8_t>(view, 2)]);
}
#endif

} // namespace WatchySensorTools
