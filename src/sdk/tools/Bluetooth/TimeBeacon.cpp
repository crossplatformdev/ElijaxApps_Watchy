#include "BluetoothToolApps.h"
#include "WatchyUi.h"
#include "Watchy.h"

#include <BLEAdvertising.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
namespace {
void configure(BLEAdvertisementData &data, const char *&description) {
  std::string payload;
  tmElements_t currentTime{};
  Watchy::RTC.read(currentTime);
  WatchyBluetoothSupport::appendUint32(
      payload, static_cast<uint32_t>(currentUtcTime(currentTime)));
  data.setName("Watchy Time");
  data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0x1805)));
  data.setServiceData(BLEUUID(static_cast<uint16_t>(0x1805)), payload);
  description = "UTC time data";
}
} // namespace
void runTimeBeacon() {
  WatchyBluetoothSupport::runBeacon(
      WatchyBluetoothSupport::title(WatchyBluetoothSupport::Tool::TimeBeacon), configure);
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderTimeBeaconPreview(uint8_t view) {
  WatchyBluetoothSupport::drawBeacon("TIME BEACON", "UTC time data", view == 0);
}
#endif
} // namespace WatchyBluetoothTools
