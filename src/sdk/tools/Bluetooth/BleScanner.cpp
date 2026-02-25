#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runScanner(BLEScanResults &results) {
  int count = limitedCount(results);
  int indices[maximumScanDevices];
  rankBySignal(results, indices, count);
  char found[8];
  snprintf(found, sizeof(found), "%d", results.getCount());
  AppVisual::drawDataRow(42, "FOUND", found, true);
  for (int row = 0; row < min(count, 6); row++) {
    BLEAdvertisedDevice device = results.getDevice(indices[row]);
    std::string name = device.haveName() ? device.getName() : "(anonymous)";
    drawBluetoothRow(65 + row * 20, name, device.getRSSI(), row == 0);
  }
  WatchyUi::Widget::footer("SORTED BY SIGNAL  BACK EXIT");
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderScannerPreview(uint8_t) {
  const char *const names[] = {"Watchy Demo", "Heart Sensor", "Keyboard",
                               "(anonymous)", "Headphones", "Beacon Lab"};
  const int rssis[] = {-48, -57, -63, -71, -76, -82};
  AppVisual::drawDataRow(42, "FOUND", "7", true);
  for (uint8_t row = 0; row < 6; row++) {
    drawBluetoothRow(65 + row * 20, names[row], rssis[row], row == 0);
  }
  WatchyUi::Widget::footer("SORTED BY SIGNAL  BACK EXIT");
}
#endif

} // namespace WatchyBluetoothTools
