#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runServiceUuids(BLEScanResults &results) {
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 6; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveServiceUUID()) continue;
    std::string uuid = device.getServiceUUID().toString();
    char label[10];
    snprintf(label, sizeof(label), "SERVICE %u", row + 1);
    AppVisual::drawDataRow(47 + row * 21, label,
                           uuid.length() > 8 ? uuid.substr(4, 4).c_str()
                                             : uuid.c_str(), row == 0);
    row++;
  }
  if (row == 0) drawEmptyBluetooth("No service UUIDs advertised");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderServiceUuidsPreview(uint8_t) {
  AppVisual::drawDataRow(47, "SERVICE 1", "180F", true);
  AppVisual::drawDataRow(68, "SERVICE 2", "180D");
  AppVisual::drawDataRow(89, "SERVICE 3", "1812");
  AppVisual::drawDataRow(110, "SERVICE 4", "FFF0");
}
#endif

} // namespace WatchyBluetoothTools
