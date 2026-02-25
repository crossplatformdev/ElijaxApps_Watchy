#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runRadar(BLEScanResults &results) {
  int count = limitedCount(results);
  int indices[maximumScanDevices];
  rankBySignal(results, indices, count);
  drawRadarFrame();
  for (int row = 0; row < min(count, 7); row++) {
    BLEAdvertisedDevice device = results.getDevice(indices[row]);
    drawRadarSignal(row, device.getRSSI());
  }
  if (count == 0) {
    drawEmptyBluetooth("No signals in range");
  } else {
    char value[20];
    snprintf(value, sizeof(value), "%d signals", count);
    AppVisual::drawDataRow(176, "RADAR", value, true);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderRadarPreview(uint8_t) {
  const int rssis[] = {-48, -57, -63, -71, -76, -82, -88};
  drawRadarFrame();
  for (uint8_t row = 0; row < 7; row++) drawRadarSignal(row, rssis[row]);
  AppVisual::drawDataRow(176, "RADAR", "7 signals", true);
}
#endif

} // namespace WatchyBluetoothTools
