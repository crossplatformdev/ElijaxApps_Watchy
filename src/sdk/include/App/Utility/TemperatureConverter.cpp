#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw(int value) {
  beginAppDisplay("TEMPERATURE CONV");
  char input[24];
  char result[32];
  snprintf(input, sizeof(input), "%d C", value);
  AppVisual::drawMetric({12, 31, 176, 54}, "INPUT", input);
  drawConverterArrow();
  snprintf(result, sizeof(result), "%.1f F", value * 1.8f + 32.0f);
  AppVisual::drawDataRow(122, "FAHRENHEIT", result, true);
  snprintf(result, sizeof(result), "%.2f K", value + 273.15f);
  AppVisual::drawDataRow(148, "KELVIN", result);
  WatchyUi::Widget::footer("UP/DOWN VALUE  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runTemperatureConverter() { runAdjustableValue(20, 1, -273, 10000, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderTemperatureConverterPreview(uint8_t) { draw(20); }
#endif
} // namespace WatchyUtilityTools
