#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runAddresses(BLEScanResults &results) {
  int count = min(results.getCount(), 6);
  for (int row = 0; row < count; row++) {
    BLEAdvertisedDevice device = results.getDevice(row);
    drawBluetoothRow(47 + row * 20, device.getAddress().toString(),
                     device.getRSSI(), row == 0);
  }
  if (count == 0) drawEmptyBluetooth("No Bluetooth addresses found");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderAddressesPreview(uint8_t) {
  const char *const addresses[] = {"02:00:00:00:00:01", "02:00:00:00:00:02",
                                   "02:00:00:00:00:03", "02:00:00:00:00:04",
                                   "02:00:00:00:00:05", "02:00:00:00:00:06"};
  const int rssis[] = {-48, -57, -63, -71, -76, -82};
  for (uint8_t row = 0; row < 6; row++) {
    drawBluetoothRow(47 + row * 20, addresses[row], rssis[row], row == 0);
  }
}
#endif

} // namespace WatchyBluetoothTools
