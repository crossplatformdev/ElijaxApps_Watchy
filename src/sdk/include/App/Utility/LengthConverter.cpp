#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw(int value) {
  beginAppDisplay("LENGTH CONVERTER");
  char input[24];
  char result[32];
  snprintf(input, sizeof(input), "%d m", value);
  AppVisual::drawMetric({12, 31, 176, 54}, "INPUT", input);
  drawConverterArrow();
  snprintf(result, sizeof(result), "%.2f ft", value * 3.28084f);
  AppVisual::drawDataRow(118, "FEET", result, true);
  snprintf(result, sizeof(result), "%.2f yd", value * 1.09361f);
  AppVisual::drawDataRow(138, "YARDS", result);
  snprintf(result, sizeof(result), "%.3f km", value / 1000.0f);
  AppVisual::drawDataRow(158, "KILOMETERS", result);
  WatchyUi::Widget::footer("UP/DOWN VALUE  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runLengthConverter() { runAdjustableValue(20, 1, 0, 10000, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderLengthConverterPreview(uint8_t) { draw(20); }
#endif
} // namespace WatchyUtilityTools
