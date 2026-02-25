#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"
#include "AppDisplay.h"
#include "BatteryModel.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawChargeStatus(const WatchyBattery::Estimate &battery, bool usbPresent,
                      bool charging) {
  char level[8];
  char voltage[16];
  snprintf(level, sizeof(level), "%u%%", battery.percent);
  snprintf(voltage, sizeof(voltage), "%.3f V", battery.voltage);
  AppVisual::drawStatusIcon({78, 37, 44, 44},
                            charging ? AppVisual::StatusIcon::SUCCESS
                                     : usbPresent ? AppVisual::StatusIcon::INFO
                                                  : AppVisual::StatusIcon::EMPTY,
                            true);
  WatchyUi::Canvas::centeredText(
      {0, 89, 200, 20}, charging ? "CHARGING"
                                 : usbPresent ? "CONNECTED" : "NO USB",
      2, WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(133, "CHARGER", charging ? "Active" : "Idle", true);
  AppVisual::drawDataRow(155, "BATTERY", level);
  AppVisual::drawDataRow(177, "VOLTAGE", voltage);
}

void draw() {
  WatchyBattery::Estimate battery =
      WatchyBattery::estimate(Watchy::getBatteryVoltage());
#ifdef ARDUINO_ESP32S3_DEV
  pinMode(USB_DET_PIN, INPUT);
  pinMode(CHRG_STATUS_PIN, INPUT);
  bool usbPresent = digitalRead(USB_DET_PIN) == HIGH;
  bool charging = usbPresent && digitalRead(CHRG_STATUS_PIN) == LOW;
  drawChargeStatus(battery, usbPresent, charging);
#else
  drawChargeStatus(battery, false, false);
#endif
}

} // namespace

void runChargeStatus() {
  runStaticTool("CHARGE STATUS", draw, batteryViewRefreshIntervalMs);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderChargeStatusPreview(uint8_t view) {
  drawChargeStatus(view == 2 ? WatchyBattery::estimate(4.18f)
                             : WatchyBattery::estimate(3.82f),
                   view != 0, view == 1);
}
#endif

} // namespace WatchySensorTools
