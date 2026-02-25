#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"

namespace WatchyBluetoothTools {

void runDeviceCount(BLEScanResults &results) {
  int named = 0;
  int services = 0;
  int manufacturers = 0;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    named += device.haveName();
    services += device.haveServiceUUID();
    manufacturers += device.haveManufacturerData();
  }
  char value[12];
  snprintf(value, sizeof(value), "%d", results.getCount());
  AppVisual::drawMetric({12, 33, 176, 70}, "DEVICES FOUND", value);
  snprintf(value, sizeof(value), "%d", named);
  AppVisual::drawDataRow(124, "NAMED", value, true);
  snprintf(value, sizeof(value), "%d", services);
  AppVisual::drawDataRow(145, "SERVICES", value);
  snprintf(value, sizeof(value), "%d", manufacturers);
  AppVisual::drawDataRow(166, "MFR DATA", value);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderDeviceCountPreview(uint8_t) {
  AppVisual::drawMetric({12, 33, 176, 70}, "DEVICES FOUND", "7");
  AppVisual::drawDataRow(124, "NAMED", "6", true);
  AppVisual::drawDataRow(145, "SERVICES", "4");
  AppVisual::drawDataRow(166, "MFR DATA", "3");
}
#endif

} // namespace WatchyBluetoothTools
