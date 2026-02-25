#ifndef WATCHY_SENSOR_TOOL_APPS_H
#define WATCHY_SENSOR_TOOL_APPS_H

#include <WatchySdk.h>
#include <Watchy.h>

namespace WatchySensorTools {

enum Tool : uint8_t {
  BatteryGauge,
  PowerBudget,
  ChargeStatus,
  BmaTemperature,
  RawAccel,
  GForce,
  SpiritLevel,
  Orientation,
  MotionScore,
  StepCounter,
  StepGoal,
  WalkDistance,
  StepCalories,
  ActivityState,
  SensorStatus,
  Uptime,
  ShakeCounter,
  ToolCount
};

void runBatteryGauge();
void runPowerBudget();
void runChargeStatus();
void runBmaTemperature();
void runRawAccel();
void runGForce();
void runSpiritLevel();
void runOrientation();
void runMotionScore();
void runStepCounter();
void runStepGoal();
void runWalkDistance();
void runStepCalories();
void runActivityState();
void runSensorStatus();
void runUptime(Watchy *watchy);
void runShakeCounter();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderBatteryGaugePreview(uint8_t view);
void renderPowerBudgetPreview(uint8_t view);
void renderChargeStatusPreview(uint8_t view);
void renderBmaTemperaturePreview(uint8_t view);
void renderRawAccelPreview(uint8_t view);
void renderGForcePreview(uint8_t view);
void renderSpiritLevelPreview(uint8_t view);
void renderOrientationPreview(uint8_t view);
void renderMotionScorePreview(uint8_t view);
void renderStepCounterPreview(uint8_t view);
void renderStepGoalPreview(uint8_t view);
void renderWalkDistancePreview(uint8_t view);
void renderStepCaloriesPreview(uint8_t view);
void renderActivityStatePreview(uint8_t view);
void renderSensorStatusPreview(uint8_t view);
void renderUptimePreview(uint8_t view);
void renderShakeCounterPreview(uint8_t view);
#endif

} // namespace WatchySensorTools

#endif