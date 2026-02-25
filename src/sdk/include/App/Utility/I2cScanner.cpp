#include "UtilityToolApps.h"

#include "WatchyUi.h"

#include "AppDisplay.h"

namespace WatchyUtilityTools {
namespace {
void draw() {
  beginAppDisplay("I2C SCANNER");
  uint8_t found = 0;
  char addresses[40] = {};
  uint8_t stored = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      if (stored < 5) {
        char value[6];
        snprintf(value, sizeof(value), "0x%02X ", address);
        strlcat(addresses, value, sizeof(addresses));
        stored++;
      }
      found++;
    }
  }
  if (found == 0) {
    AppVisual::drawEmptyState({8, 38, 184, 118}, "NO I2C DEVICES",
                              "No address acknowledged");
  } else {
    char count[8];
    snprintf(count, sizeof(count), "%u", found);
    AppVisual::drawMetric({12, 34, 176, 74}, "RESPONDING ADDRESSES", count);
    AppVisual::drawDataRow(136, "FOUND", addresses, true);
    AppVisual::drawDataRow(164, "MODE", "Read-only probe");
  }
  WatchyUi::Widget::footer("BACK EXIT");
  finishAppDisplay();
}
} // namespace
void runI2cScanner() { draw(); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderI2cScannerPreview(uint8_t view) {
  beginAppDisplay("I2C SCANNER");
  if (view == 0) {
    AppVisual::drawMetric({12, 34, 176, 74}, "RESPONDING ADDRESSES", "2");
    AppVisual::drawDataRow(136, "FOUND", "0x18  0x51", true);
    AppVisual::drawDataRow(164, "MODE", "Read-only probe");
  } else {
    AppVisual::drawEmptyState({8, 38, 184, 118}, "NO I2C DEVICES",
                              "No address acknowledged");
  }
  WatchyUi::Widget::footer("BACK EXIT");
  finishAppDisplay();
}
#endif
} // namespace WatchyUtilityTools
