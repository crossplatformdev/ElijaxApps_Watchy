#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw(int value) {
  beginAppDisplay("BASE CONVERTER");
  char input[24];
  char result[32];
  snprintf(input, sizeof(input), "%d", value);
  AppVisual::drawMetric({12, 31, 176, 54}, "DECIMAL", input);
  drawConverterArrow();
  snprintf(result, sizeof(result), "0x%X", value);
  AppVisual::drawDataRow(118, "HEX", result, true);
  snprintf(result, sizeof(result), "0%o", value);
  AppVisual::drawDataRow(138, "OCTAL", result);
  snprintf(result, sizeof(result), "%s", String(value, BIN).c_str());
  AppVisual::drawDataRow(158, "BINARY", result);
  WatchyUi::Widget::footer("UP/DOWN VALUE  BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runBaseConverter() { runAdjustableValue(42, 1, 0, 65535, draw); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBaseConverterPreview(uint8_t) { draw(42); }
#endif
} // namespace WatchyUtilityTools
