#include "SensorToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "SensorManager.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawStatus(bool accelerationEnabled, uint8_t status, uint8_t error) {
  char statusText[8];
  char errorText[8];
  snprintf(statusText, sizeof(statusText), "0x%02X", status);
  snprintf(errorText, sizeof(errorText), "0x%02X", error);
  AppVisual::drawStatusIcon({79, 36, 42, 42},
                            error == 0 ? AppVisual::StatusIcon::SUCCESS
                                       : AppVisual::StatusIcon::WARNING,
                            true);
  WatchyUi::Canvas::centeredText(
      {0, 89, 200, 20}, error == 0 ? "SENSOR HEALTHY" : "CHECK SENSOR", 2,
      WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(133, "ACCEL", accelerationEnabled ? "Enabled" : "Disabled",
                         true);
  AppVisual::drawDataRow(153, "STATUS", statusText);
  AppVisual::drawDataRow(173, "ERROR", errorText);
}

void draw() {
  WatchySensor::Diagnostics diagnostics = WatchySensor::readDiagnostics();
  drawStatus(diagnostics.accelerationEnabled, diagnostics.status,
             diagnostics.error);
}

} // namespace

void runSensorStatus() {
  runStaticTool("SENSOR STATUS", draw);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderSensorStatusPreview(uint8_t view) {
  drawStatus(view == 0, view == 0 ? 0x81 : 0x00, view == 0 ? 0x00 : 0x02);
}
#endif

} // namespace WatchySensorTools
