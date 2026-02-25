#include "BluetoothToolApps.h"

#include "WatchyUi.h"

#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>

#include "AppDisplay.h"

namespace WatchyBluetoothTools {

void runTxPower(BLEScanResults &results) {
  int count = 0;
  int total = 0;
  int minimum = 127;
  int maximum = -127;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveTXPower()) continue;
    int power = device.getTXPower();
    total += power;
    minimum = min(minimum, power);
    maximum = max(maximum, power);
    count++;
  }
  char value[12];
  snprintf(value, sizeof(value), "%d", count);
  AppVisual::drawMetric({12, 34, 176, 70}, "TX POWER REPORTS", value);
  if (count > 0) {
    snprintf(value, sizeof(value), "%d dBm", total / count);
    AppVisual::drawDataRow(130, "AVERAGE", value, true);
    snprintf(value, sizeof(value), "%d / %d", minimum, maximum);
    AppVisual::drawDataRow(155, "MIN / MAX", value);
  } else {
    AppVisual::drawEmptyState({8, 116, 184, 55}, "NO TX POWER", nullptr);
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderTxPowerPreview(uint8_t) {
  AppVisual::drawMetric({12, 34, 176, 70}, "TX POWER REPORTS", "4");
  AppVisual::drawDataRow(130, "AVERAGE", "-4 dBm", true);
  AppVisual::drawDataRow(155, "MIN / MAX", "-12 / 4");
}
#endif

} // namespace WatchyBluetoothTools
