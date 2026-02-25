#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw(int value) {
  beginAppDisplay("PACE CONVERTER");
  char input[24];
  char result[32];
  snprintf(input, sizeof(input), "%d:%02d /km", value / 60, value % 60);
  AppVisual::drawMetric({12, 31, 176, 54}, "PACE", input);
  drawConverterArrow();
  float kilometersPerHour = 3600.0f / value;
  float minutesPerMile = value * 1.609344f / 60.0f;
  snprintf(result, sizeof(result), "%.2f km/h", kilometersPerHour);
  AppVisual::drawDataRow(122, "SPEED", result, true);
  snprintf(result, sizeof(result), "%.2f min/mi", minutesPerMile);
  AppVisual::drawDataRow(148, "MILE PACE", result);
  WatchyUi::Widget::footer("UP/DOWN VALUE  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runPaceConverter() { runAdjustableValue(300, 5, 60, 1200, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderPaceConverterPreview(uint8_t) { draw(300); }
#endif
} // namespace WatchyUtilityTools
