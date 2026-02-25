#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw(int value) {
  beginAppDisplay("WEIGHT CONVERTER");
  char input[24];
  char result[32];
  snprintf(input, sizeof(input), "%d kg", value);
  AppVisual::drawMetric({12, 31, 176, 54}, "INPUT", input);
  drawConverterArrow();
  snprintf(result, sizeof(result), "%.2f lb", value * 2.20462f);
  AppVisual::drawDataRow(122, "POUNDS", result, true);
  snprintf(result, sizeof(result), "%.1f oz", value * 35.274f);
  AppVisual::drawDataRow(148, "OUNCES", result);
  WatchyUi::Widget::footer("UP/DOWN VALUE  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runWeightConverter() { runAdjustableValue(20, 1, 0, 10000, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderWeightConverterPreview(uint8_t) { draw(20); }
#endif
} // namespace WatchyUtilityTools
