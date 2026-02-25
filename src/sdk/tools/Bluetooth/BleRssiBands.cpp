#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEScan.h>

#include "AppDisplay.h"

namespace WatchyBluetoothTools {

void runRssiBands(BLEScanResults &results) {
  int close = 0;
  int near = 0;
  int far = 0;
  for (int index = 0; index < results.getCount(); index++) {
    int rssi = results.getDevice(index).getRSSI();
    if (rssi >= -60) close++;
    else if (rssi >= -80) near++;
    else far++;
  }
  char value[12];
  AppVisual::drawSignalBars({16, 39, 50, 54}, 4, 4, true);
  snprintf(value, sizeof(value), "%d", close);
  AppVisual::drawDataRow(119, "CLOSE / -60", value, true);
  snprintf(value, sizeof(value), "%d", near);
  AppVisual::drawDataRow(142, "NEAR / -80", value);
  snprintf(value, sizeof(value), "%d", far);
  AppVisual::drawDataRow(165, "FAR", value);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderRssiBandsPreview(uint8_t) {
  AppVisual::drawSignalBars({16, 39, 50, 54}, 4, 4, true);
  AppVisual::drawDataRow(119, "CLOSE / -60", "2", true);
  AppVisual::drawDataRow(142, "NEAR / -80", "3");
  AppVisual::drawDataRow(165, "FAR", "2");
}
#endif

} // namespace WatchyBluetoothTools
