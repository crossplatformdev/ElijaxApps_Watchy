#include "SensorToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"
#include "AppDisplay.h"
#include "BatteryModel.h"
#include "SensorSupport.h"

namespace WatchySensorTools {
namespace {

constexpr float averageLoadMa = 0.4f;

void drawPowerBudget(const WatchyBattery::Estimate &battery) {
  float estimatedDays = battery.chargeMah / averageLoadMa / 24.0f;
  char days[12];
  char charge[20];
  snprintf(days, sizeof(days), "%.1f d", estimatedDays);
  snprintf(charge, sizeof(charge), "%.0f mAh", battery.chargeMah);
  AppVisual::drawMetric({12, 34, 176, 82}, "IDLE ESTIMATE", days,
                        battery.percent / 100.0f,
                        "At the modeled average load");
  AppVisual::drawDataRow(138, "CHARGE", charge, true);
  AppVisual::drawDataRow(160, "AVERAGE LOAD", "0.4 mA");
  AppVisual::drawDataRow(182, "MODEL", "200 mAh LiPo");
}

void draw() {
  drawPowerBudget(WatchyBattery::estimate(Watchy::getBatteryVoltage()));
}

} // namespace

void runPowerBudget() {
  runStaticTool("POWER BUDGET", draw, batteryViewRefreshIntervalMs);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderPowerBudgetPreview(uint8_t) {
  drawPowerBudget(WatchyBattery::estimate(3.82f));
}
#endif

} // namespace WatchySensorTools
