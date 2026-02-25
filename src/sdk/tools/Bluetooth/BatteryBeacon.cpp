#include "BluetoothToolApps.h"
#include "WatchyUi.h"
#include "Watchy.h"


#include <BLEAdvertising.h>

#include "BatteryModel.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
namespace {
void configure(BLEAdvertisementData &data, const char *&description) {
  std::string payload;
  payload.push_back(static_cast<char>(
      WatchyBattery::estimate(Watchy::getBatteryVoltage()).percent));
  data.setName("Watchy Battery");
  data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0x180F)));
  data.setServiceData(BLEUUID(static_cast<uint16_t>(0x180F)), payload);
  description = "Battery service";
}
} // namespace
void runBatteryBeacon() {
  WatchyBluetoothSupport::runBeacon(
      WatchyBluetoothSupport::title(WatchyBluetoothSupport::Tool::BatteryBeacon), configure);
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBatteryBeaconPreview(uint8_t view) {
  WatchyBluetoothSupport::drawBeacon("BATTERY BEACON", "Battery service", view == 0);
}
#endif
} // namespace WatchyBluetoothTools
