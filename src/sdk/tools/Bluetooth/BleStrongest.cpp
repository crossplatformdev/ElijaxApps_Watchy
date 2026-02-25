#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
using namespace WatchyBluetoothSupport;

void runStrongest(BLEScanResults &results) {
  if (results.getCount() == 0) {
    drawEmptyBluetooth("Scan completed with no signals");
    return;
  }
  BLEAdvertisedDevice strongest = results.getDevice(0);
  for (int index = 1; index < results.getCount(); index++) {
    BLEAdvertisedDevice candidate = results.getDevice(index);
    if (candidate.getRSSI() > strongest.getRSSI()) strongest = candidate;
  }
  std::string name = strongest.haveName() ? strongest.getName() : "Anonymous";
  char rssi[12];
  snprintf(rssi, sizeof(rssi), "%d dBm", strongest.getRSSI());
  AppVisual::drawMetric({12, 33, 116, 78}, "STRONGEST", rssi);
  AppVisual::drawSignalBars({137, 42, 48, 62},
                            signalStrength(strongest.getRSSI()), 4, true);
  AppVisual::drawDataRow(137, "DEVICE", shortened(name, 16).c_str(), true);
  AppVisual::drawDataRow(163, "ADDRESS", strongest.getAddress().toString().c_str());
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStrongestPreview(uint8_t) {
  AppVisual::drawMetric({12, 33, 116, 78}, "STRONGEST", "-48 dBm");
  AppVisual::drawSignalBars({137, 42, 48, 62}, 4, 4, true);
  AppVisual::drawDataRow(137, "DEVICE", "Watchy Demo", true);
  AppVisual::drawDataRow(163, "ADDRESS", "02:00:00:00:00:01");
}
#endif

} // namespace WatchyBluetoothTools
