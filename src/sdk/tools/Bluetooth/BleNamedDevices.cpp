#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runNamedDevices(BLEScanResults &results) {
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 6; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveName()) continue;
    drawBluetoothRow(47 + row * 20, device.getName(), device.getRSSI(), row == 0);
    row++;
  }
  if (row == 0) drawEmptyBluetooth("No advertised names in this scan");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderNamedDevicesPreview(uint8_t) {
  const char *const names[] = {"Watchy Demo", "Heart Sensor", "Keyboard",
                               "Headphones", "Beacon Lab", "Tablet"};
  const int rssis[] = {-48, -57, -63, -76, -82, -88};
  for (uint8_t row = 0; row < 6; row++) {
    drawBluetoothRow(47 + row * 20, names[row], rssis[row], row == 0);
  }
}
#endif

} // namespace WatchyBluetoothTools
