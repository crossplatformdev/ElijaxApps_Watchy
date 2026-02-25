#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"

namespace WatchyBluetoothTools {

void runIBeacons(BLEScanResults &results) {
  int ibeacons = 0;
  int applePackets = 0;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveManufacturerData()) continue;
    std::string data = device.getManufacturerData();
    bool apple = data.size() >= 2 && static_cast<uint8_t>(data[0]) == 0x4c &&
                 static_cast<uint8_t>(data[1]) == 0x00;
    applePackets += apple;
    ibeacons += apple && data.size() >= 4 &&
                static_cast<uint8_t>(data[2]) == 0x02 &&
                static_cast<uint8_t>(data[3]) == 0x15;
  }
  char value[12];
  snprintf(value, sizeof(value), "%d", ibeacons);
  AppVisual::drawMetric({12, 39, 176, 90}, "IBEACON PACKETS", value);
  snprintf(value, sizeof(value), "%d", applePackets);
  AppVisual::drawDataRow(165, "APPLE PACKETS", value, true);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderIBeaconsPreview(uint8_t) {
  AppVisual::drawMetric({12, 39, 176, 90}, "IBEACON PACKETS", "2");
  AppVisual::drawDataRow(165, "APPLE PACKETS", "3", true);
}
#endif

} // namespace WatchyBluetoothTools
