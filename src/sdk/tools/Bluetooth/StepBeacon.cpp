#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertising.h>

#include "BluetoothSupport.h"
#include "SensorManager.h"

namespace WatchyBluetoothTools {
namespace {
void configure(BLEAdvertisementData &data, const char *&description) {
  std::string payload;
  WatchyBluetoothSupport::appendUint32(payload, WatchySensor::stepCount());
  data.setName("Watchy Steps");
  data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0xFFF0)));
  data.setServiceData(BLEUUID(static_cast<uint16_t>(0xFFF0)), payload);
  description = "Step data";
}
} // namespace
void runStepBeacon() {
  WatchyBluetoothSupport::runBeacon(
      WatchyBluetoothSupport::title(WatchyBluetoothSupport::Tool::StepBeacon), configure);
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderStepBeaconPreview(uint8_t view) {
  WatchyBluetoothSupport::drawBeacon("STEP BEACON", "Step data", view == 0);
}
#endif
} // namespace WatchyBluetoothTools
