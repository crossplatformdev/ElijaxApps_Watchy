#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertising.h>

#include "BluetoothSupport.h"

namespace WatchyBluetoothTools {
namespace {
void configure(BLEAdvertisementData &data, const char *&description) {
  data.setName("WATCHY BADGE");
  description = "Device name";
}
} // namespace
void runNameBadge() {
  WatchyBluetoothSupport::runBeacon(
      WatchyBluetoothSupport::title(WatchyBluetoothSupport::Tool::NameBadge), configure);
}
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderNameBadgePreview(uint8_t view) {
  WatchyBluetoothSupport::drawBeacon("NAME BADGE", "Device name", view == 0);
}
#endif
} // namespace WatchyBluetoothTools
