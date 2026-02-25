#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"
#include "AppDisplay.h"
#include "BatteryModel.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

void drawBatteryShape(const WatchyBattery::Estimate &battery) {
  constexpr WatchyUi::Bounds body{132, 53, 47, 30};
  const uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.drawRoundRect(body.x, body.y, body.width, body.height, 4,
                                foreground);
  Watchy::display.fillRect(body.x + body.width, body.y + 10, 4, 10,
                            foreground);
  int16_t fillWidth = static_cast<int16_t>(
      (body.width - 8) * constrain(battery.percent, 0, 100) / 100.0f);
  Watchy::display.fillRect(body.x + 4, body.y + 4, fillWidth,
                            body.height - 8, foreground);
}

void drawBatteryGauge(const WatchyBattery::Estimate &battery) {
  char value[8];
  char voltage[16];
  char charge[20];
  snprintf(value, sizeof(value), "%u%%", battery.percent);
  snprintf(voltage, sizeof(voltage), "%.3f V", battery.voltage);
  snprintf(charge, sizeof(charge), "%.0f / %.0f mAh", battery.chargeMah,
           WATCHY_DEFAULT_BATTERY_CAPACITY_MAH);
  AppVisual::drawMetric({12, 34, 108, 89}, "BATTERY", value,
                        battery.percent / 100.0f, "Estimated charge");
  drawBatteryShape(battery);
  AppVisual::drawDataRow(151, "VOLTAGE", voltage, true);
  AppVisual::drawDataRow(175, "CAPACITY", charge);
}

void draw() {
  drawBatteryGauge(WatchyBattery::estimate(Watchy::getBatteryVoltage()));
}

} // namespace

void runBatteryGauge() {
  runStaticTool("BATTERY GAUGE", draw, batteryViewRefreshIntervalMs);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBatteryGaugePreview(uint8_t) {
  drawBatteryGauge(WatchyBattery::estimate(3.82f));
}
#endif

} // namespace WatchySensorTools
