#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runManufacturers(BLEScanResults &results) {
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 6; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveManufacturerData()) continue;
    std::string data = device.getManufacturerData();
    uint16_t company = data.size() >= 2
        ? static_cast<uint8_t>(data[0]) |
              static_cast<uint16_t>(static_cast<uint8_t>(data[1])) << 8
        : 0;
    char identifier[14];
    snprintf(identifier, sizeof(identifier), "0x%04X / %u B", company,
             static_cast<unsigned int>(data.size()));
    AppVisual::drawDataRow(47 + row * 21, "MANUFACTURER", identifier, row == 0);
    row++;
  }
  if (row == 0) drawEmptyBluetooth("No manufacturer packets seen");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderManufacturersPreview(uint8_t) {
  AppVisual::drawDataRow(47, "MANUFACTURER", "0x004C / 27 B", true);
  AppVisual::drawDataRow(68, "MANUFACTURER", "0x0006 / 18 B");
  AppVisual::drawDataRow(89, "MANUFACTURER", "0x0131 / 12 B");
}
#endif

} // namespace WatchyBluetoothTools
