#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "SensorToolApps.h"

namespace {

void showSensorToolImpl(uint8_t rawTool, Watchy *watchy) {
  using namespace WatchySensorTools;
  switch (rawTool < ToolCount ? static_cast<Tool>(rawTool) : BatteryGauge) {
  case BatteryGauge: runBatteryGauge(); break;
  case PowerBudget: runPowerBudget(); break;
  case ChargeStatus: runChargeStatus(); break;
  case BmaTemperature: runBmaTemperature(); break;
  case RawAccel: runRawAccel(); break;
  case GForce: runGForce(); break;
  case SpiritLevel: runSpiritLevel(); break;
  case Orientation: runOrientation(); break;
  case MotionScore: runMotionScore(); break;
  case StepCounter: runStepCounter(); break;
  case StepGoal: runStepGoal(); break;
  case WalkDistance: runWalkDistance(); break;
  case StepCalories: runStepCalories(); break;
  case ActivityState: runActivityState(); break;
  case SensorStatus: runSensorStatus(); break;
  case Uptime: runUptime(watchy); break;
  case ShakeCounter: runShakeCounter(); break;
  default: break;
  }
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

} // namespace

void Watchy::showSensorTool(uint8_t rawTool) {
  showSensorToolImpl(rawTool, this);
}

void WatchySdk::showSensorTool(uint8_t rawTool) {
  showSensorToolImpl(rawTool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSensorPreview(uint8_t rawTool, uint8_t view) {
  using namespace WatchySensorTools;
  Tool tool = rawTool < ToolCount ? static_cast<Tool>(rawTool) : BatteryGauge;
  if (tool == ShakeCounter) {
    renderShakeCounterPreview(view);
    return;
  }
  beginAppDisplay(tool == BatteryGauge ? "BATTERY GAUGE"
                  : tool == PowerBudget ? "POWER BUDGET"
                  : tool == ChargeStatus ? "CHARGE STATUS"
                  : tool == BmaTemperature ? "BMA TEMPERATURE"
                  : tool == RawAccel ? "RAW ACCEL"
                  : tool == GForce ? "G FORCE"
                  : tool == SpiritLevel ? "SPIRIT LEVEL"
                  : tool == Orientation ? "ORIENTATION"
                  : tool == MotionScore ? "MOTION SCORE"
                  : tool == StepCounter ? "STEP COUNTER"
                  : tool == StepGoal ? "STEP GOAL"
                  : tool == WalkDistance ? "WALK DISTANCE"
                  : tool == StepCalories ? "STEP CALORIES"
                  : tool == ActivityState ? "ACTIVITY"
                  : tool == SensorStatus ? "SENSOR STATUS" : "UPTIME");
  switch (tool) {
  case BatteryGauge: renderBatteryGaugePreview(view); break;
  case PowerBudget: renderPowerBudgetPreview(view); break;
  case ChargeStatus: renderChargeStatusPreview(view); break;
  case BmaTemperature: renderBmaTemperaturePreview(view); break;
  case RawAccel: renderRawAccelPreview(view); break;
  case GForce: renderGForcePreview(view); break;
  case SpiritLevel: renderSpiritLevelPreview(view); break;
  case Orientation: renderOrientationPreview(view); break;
  case MotionScore: renderMotionScorePreview(view); break;
  case StepCounter: renderStepCounterPreview(view); break;
  case StepGoal: renderStepGoalPreview(view); break;
  case WalkDistance: renderWalkDistancePreview(view); break;
  case StepCalories: renderStepCaloriesPreview(view); break;
  case ActivityState: renderActivityStatePreview(view); break;
  case SensorStatus: renderSensorStatusPreview(view); break;
  case Uptime: renderUptimePreview(view); break;
  default: break;
  }
  finishAppDisplay();
}

} // namespace WatchyDemo
#endif
