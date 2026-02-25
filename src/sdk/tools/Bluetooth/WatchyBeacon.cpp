#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertising.h>

#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
namespace {
void configure(BLEAdvertisementData &data, const char *&description) {
  std::string payload("WY", 2);
  payload.push_back(1);
  data.setName("Watchy");
  data.setManufacturerData(payload);
  description = "Watchy ID";
}
} // namespace
void runWatchyBeacon() {
  WatchyBluetoothSupport::runBeacon(
      WatchyBluetoothSupport::title(WatchyBluetoothSupport::Tool::Beacon), configure);
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderWatchyBeaconPreview(uint8_t view) {
  WatchyBluetoothSupport::drawBeacon("WATCHY BEACON", "Watchy ID", view == 0);
}
#endif
} // namespace WatchyBluetoothTools
