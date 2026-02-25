#include <Watchy.h>
#include <math.h>
#include "AppDisplay.h"
#include "BatteryModel.h"

extern RTC_DATA_ATTR tmElements_t bootTime;

namespace {

enum SensorTool : uint8_t {
  BATTERY_GAUGE,
  POWER_BUDGET,
  CHARGE_STATUS,
  BMA_TEMPERATURE,
  RAW_ACCEL,
  G_FORCE,
  SPIRIT_LEVEL,
  ORIENTATION,
  MOTION_SCORE,
  STEP_COUNTER,
  STEP_GOAL,
  WALK_DISTANCE,
  STEP_CALORIES,
  ACTIVITY_STATE,
  SENSOR_STATUS,
  UPTIME,
  SHAKE_COUNTER,
  SENSOR_TOOL_COUNT
};

const char *const titles[SENSOR_TOOL_COUNT] = {
    "BATTERY GAUGE", "POWER BUDGET", "CHARGE STATUS", "BMA TEMPERATURE",
    "RAW ACCEL",     "G FORCE",      "SPIRIT LEVEL",  "ORIENTATION",
    "MOTION SCORE",  "STEP COUNTER", "STEP GOAL",     "WALK DISTANCE",
    "STEP CALORIES", "ACTIVITY",     "SENSOR STATUS", "UPTIME",
    "SHAKE COUNTER"};

constexpr float ACCEL_LSB_PER_G = 1024.0f;
constexpr uint32_t DAILY_STEP_GOAL = 10000;
constexpr float POWER_BUDGET_AVERAGE_LOAD_MA = 0.4f;

void useBodyText(int16_t x = 4, int16_t y = 42) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void printBatteryCharge(const WatchyBattery::Estimate &battery) {
  Watchy::display.print(battery.chargeMah, 0);
  Watchy::display.print('/');
  Watchy::display.print(WATCHY_DEFAULT_BATTERY_CAPACITY_MAH, 0);
  Watchy::display.print(" mAh");
}

bool readAcceleration(Accel &acceleration) {
  return sensor.getAccel(acceleration);
}

float accelerationMagnitude(const Accel &acceleration) {
  float x = acceleration.x / ACCEL_LSB_PER_G;
  float y = acceleration.y / ACCEL_LSB_PER_G;
  float z = acceleration.z / ACCEL_LSB_PER_G;
  return sqrtf(x * x + y * y + z * z);
}

const char *orientationName(uint8_t direction) {
  switch (direction) {
  case DIRECTION_DISP_DOWN: return "FACE DOWN";
  case DIRECTION_DISP_UP: return "FACE UP";
  case DIRECTION_BOTTOM_EDGE: return "BOTTOM EDGE";
  case DIRECTION_TOP_EDGE: return "TOP EDGE";
  case DIRECTION_RIGHT_EDGE: return "RIGHT EDGE";
  case DIRECTION_LEFT_EDGE: return "LEFT EDGE";
  default: return "UNKNOWN";
  }
}

const char *activityName(const char *rawActivity) {
  if (strstr(rawActivity, "STATIONARY") != nullptr) {
    return "STATIONARY";
  }
  if (strstr(rawActivity, "WALKING") != nullptr) {
    return "WALKING";
  }
  if (strstr(rawActivity, "RUNNING") != nullptr) {
    return "RUNNING";
  }
  return "UNKNOWN";
}

void drawBatteryGauge(const WatchyBattery::Estimate &battery) {
  useBodyText(8, 55);
  Watchy::display.setTextSize(4);
  Watchy::display.print(battery.percent);
  Watchy::display.println('%');
  WatchyUi::Widget::progress(battery.percent / 100.0f, 112);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 158);
  Watchy::display.print(battery.voltage, 3);
  Watchy::display.println(" V LiPo");
  Watchy::display.setCursor(8, 180);
  Watchy::display.print("CHARGE ");
  printBatteryCharge(battery);
}

void drawBatteryGauge(Watchy &watch) {
  drawBatteryGauge(WatchyBattery::estimate(watch.getBatteryVoltage()));
}

void drawPowerBudget(const WatchyBattery::Estimate &battery) {
  float estimatedDays =
      battery.chargeMah / POWER_BUDGET_AVERAGE_LOAD_MA / 24.0f;
  useBodyText(6, 47);
  Watchy::display.print("CHARGE EST.  ");
  Watchy::display.print(battery.percent);
  Watchy::display.println('%');
  Watchy::display.setCursor(6, 75);
  Watchy::display.print("ENERGY EST.  ");
  printBatteryCharge(battery);
  Watchy::display.println();
  Watchy::display.setCursor(6, 103);
  Watchy::display.print("IDLE EST.    ");
  Watchy::display.print(estimatedDays, 1);
  Watchy::display.println(" days");
  Watchy::display.setCursor(6, 145);
  Watchy::display.print("Model: ");
  Watchy::display.print(WATCHY_DEFAULT_BATTERY_CAPACITY_MAH, 0);
  Watchy::display.println("mAh battery");
  Watchy::display.setCursor(6, 162);
  Watchy::display.print("at ");
  Watchy::display.print(POWER_BUDGET_AVERAGE_LOAD_MA, 1);
  Watchy::display.println("mA average load");
}

void drawPowerBudget(Watchy &watch) {
  drawPowerBudget(WatchyBattery::estimate(watch.getBatteryVoltage()));
}

void drawChargeStatus(const WatchyBattery::Estimate &battery,
                      bool usbPresent, bool charging) {
  useBodyText(12, 52);
  Watchy::display.print("USB       ");
  Watchy::display.println(usbPresent ? "CONNECTED" : "ABSENT");
  Watchy::display.setCursor(12, 82);
  Watchy::display.print("CHARGER   ");
  Watchy::display.println(charging ? "CHARGING" : "IDLE");
  Watchy::display.setCursor(12, 116);
  Watchy::display.print("BATTERY   ");
  Watchy::display.print(battery.voltage, 3);
  Watchy::display.println(" V");
  Watchy::display.setCursor(12, 150);
  Watchy::display.print("LEVEL     ");
  Watchy::display.print(battery.percent);
  Watchy::display.println('%');
  Watchy::display.setCursor(12, 178);
  Watchy::display.print("CHARGE    ");
  printBatteryCharge(battery);
}

void drawChargeStatus(Watchy &watch) {
  WatchyBattery::Estimate battery =
      WatchyBattery::estimate(watch.getBatteryVoltage());
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

void drawTemperature(float celsius) {
  useBodyText(25, 60);
  Watchy::display.setTextSize(3);
  Watchy::display.print(celsius, 1);
  Watchy::display.println(" C");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(25, 120);
  Watchy::display.print(celsius * 1.8f + 32.0f, 1);
  Watchy::display.println(" F");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 165);
  Watchy::display.println("Internal sensor value");
}

void drawTemperature() {
  drawTemperature(sensor.readTemperature());
}

void drawRawAcceleration(const Accel &acceleration) {
  useBodyText(12, 52);
  Watchy::display.print("X RAW  ");
  Watchy::display.println(acceleration.x);
  Watchy::display.setCursor(12, 82);
  Watchy::display.print("Y RAW  ");
  Watchy::display.println(acceleration.y);
  Watchy::display.setCursor(12, 112);
  Watchy::display.print("Z RAW  ");
  Watchy::display.println(acceleration.z);
  Watchy::display.setCursor(12, 155);
  Watchy::display.println("Scale: 1024 LSB/g");
}

void drawGForce(const Accel &acceleration) {
  float x = acceleration.x / ACCEL_LSB_PER_G;
  float y = acceleration.y / ACCEL_LSB_PER_G;
  float z = acceleration.z / ACCEL_LSB_PER_G;
  useBodyText(8, 48);
  Watchy::display.print("X "); Watchy::display.print(x, 2); Watchy::display.println(" g");
  Watchy::display.setCursor(8, 72);
  Watchy::display.print("Y "); Watchy::display.print(y, 2); Watchy::display.println(" g");
  Watchy::display.setCursor(8, 96);
  Watchy::display.print("Z "); Watchy::display.print(z, 2); Watchy::display.println(" g");
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(8, 142);
  Watchy::display.print(accelerationMagnitude(acceleration), 2);
  Watchy::display.println(" g");
}

void drawLevel(const Accel &acceleration) {
  float x = acceleration.x / ACCEL_LSB_PER_G;
  float y = acceleration.y / ACCEL_LSB_PER_G;
  float z = acceleration.z / ACCEL_LSB_PER_G;
  float roll = atan2f(y, z) * 180.0f / PI;
  float pitch = atan2f(-x, sqrtf(y * y + z * z)) * 180.0f / PI;
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  Watchy::display.drawCircle(100, 105, 58, color);
  Watchy::display.drawLine(42, 105, 158, 105, color);
  Watchy::display.drawLine(100, 47, 100, 163, color);
  int16_t dotX = 100 + constrain(static_cast<int>(roll), -50, 50);
  int16_t dotY = 105 + constrain(static_cast<int>(pitch), -50, 50);
  Watchy::display.fillCircle(dotX, dotY, 5, color);
  useBodyText(4, 184);
  Watchy::display.print("ROLL "); Watchy::display.print(roll, 0);
  Watchy::display.print("  PITCH "); Watchy::display.print(pitch, 0);
}

void drawOrientation(const char *name) {
  useBodyText(12, 66);
  Watchy::display.setTextSize(2);
  Watchy::display.println(name);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 118);
  Watchy::display.println("BMA423 dominant axis");
}

void drawOrientation() {
  drawOrientation(orientationName(sensor.getDirection()));
}

void drawMotionScore(float score) {
  useBodyText(20, 62);
  Watchy::display.setTextSize(3);
  Watchy::display.println(score, 3);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(20, 112);
  Watchy::display.println("mean delta-g / sample");
  Watchy::display.setCursor(20, 145);
  Watchy::display.println(score < 0.03f ? "STILL" : score < 0.15f ? "MOVING" : "ACTIVE");
}

void drawMotionScore() {
  Accel previous = {};
  Accel current = {};
  if (!readAcceleration(previous)) {
    useBodyText();
    Watchy::display.println("Sensor read failed");
    return;
  }
  float totalDelta = 0.0f;
  for (uint8_t sample = 0; sample < 40; sample++) {
    delay(20);
    if (readAcceleration(current)) {
      float dx = current.x - previous.x;
      float dy = current.y - previous.y;
      float dz = current.z - previous.z;
      totalDelta += sqrtf(dx * dx + dy * dy + dz * dz) / ACCEL_LSB_PER_G;
      previous = current;
    }
  }
  drawMotionScore(totalDelta / 40.0f);
}

void drawSteps(uint32_t steps) {
  useBodyText(18, 64);
  Watchy::display.setTextSize(4);
  Watchy::display.println(steps);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(18, 128);
  Watchy::display.println("BMA423 step counter");
}

void drawStepGoal(uint32_t steps) {
  float progress = static_cast<float>(steps) / DAILY_STEP_GOAL;
  useBodyText(12, 52);
  Watchy::display.print(steps);
  Watchy::display.print(" / ");
  Watchy::display.println(DAILY_STEP_GOAL);
  WatchyUi::Widget::progress(progress, 78);
  Watchy::display.setCursor(12, 128);
  Watchy::display.print(min(100.0f, progress * 100.0f), 1);
  Watchy::display.println("% complete");
  Watchy::display.setCursor(12, 158);
  Watchy::display.print(steps >= DAILY_STEP_GOAL ? 0 : DAILY_STEP_GOAL - steps);
  Watchy::display.println(" remaining");
}

void drawDistance(uint32_t steps) {
  float kilometers = steps * 0.00075f;
  useBodyText(18, 62);
  Watchy::display.setTextSize(3);
  Watchy::display.print(kilometers, 2);
  Watchy::display.println(" km");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(18, 122);
  Watchy::display.print(kilometers * 0.621371f, 2);
  Watchy::display.println(" miles");
  Watchy::display.setCursor(18, 158);
  Watchy::display.println("Assumes 0.75m stride");
}

void drawCalories(uint32_t steps) {
  float calories = steps * 0.04f;
  useBodyText(18, 62);
  Watchy::display.setTextSize(3);
  Watchy::display.print(calories, 0);
  Watchy::display.println(" kcal");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(18, 122);
  Watchy::display.print(steps);
  Watchy::display.println(" steps");
  Watchy::display.setCursor(18, 158);
  Watchy::display.println("Estimate: 0.04 kcal/step");
}

void drawActivity(const char *activity) {
  useBodyText(15, 70);
  Watchy::display.setTextSize(2);
  Watchy::display.println(activity);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(15, 120);
  Watchy::display.println("BMA423 classifier");
}

void drawActivity() {
  bool classifierEnabled = sensor.enableFeature(BMA423_ACTIVITY, true);
  if (classifierEnabled) {
    delay(1200);
  }
  const char *activity = classifierEnabled
                             ? activityName(sensor.getActivity())
                             : "UNAVAILABLE";
  if (classifierEnabled) {
    sensor.enableFeature(BMA423_ACTIVITY, false);
  }
  drawActivity(activity);
}

void drawSensorStatus(bool accelerationEnabled, uint8_t status,
                      uint8_t error, uint32_t sensorTime) {
  useBodyText(8, 46);
  Watchy::display.print("ACCEL     ");
  Watchy::display.println(accelerationEnabled ? "ENABLED" : "DISABLED");
  Watchy::display.setCursor(8, 72);
  Watchy::display.print("STATUS    0x");
  Watchy::display.println(status, HEX);
  Watchy::display.setCursor(8, 98);
  Watchy::display.print("ERROR     0x");
  Watchy::display.println(error, HEX);
  Watchy::display.setCursor(8, 124);
  Watchy::display.print("SENSOR T  ");
  Watchy::display.println(sensorTime);
  Watchy::display.setCursor(8, 158);
  Watchy::display.println("Range +/-2g, ODR 100Hz");
}

void drawSensorStatus() {
  drawSensorStatus(sensor.getAccelEnable(), sensor.getStatus(),
                   sensor.getErrorCode(), sensor.getSensorTime());
}

void drawUptime(uint32_t seconds) {
  uint16_t days = seconds / SECS_PER_DAY;
  uint8_t hours = seconds / SECS_PER_HOUR % 24;
  uint8_t minutes = seconds / SECS_PER_MIN % 60;
  useBodyText(12, 58);
  Watchy::display.setTextSize(2);
  Watchy::display.print(days);
  Watchy::display.print("d ");
  Watchy::display.print(hours);
  Watchy::display.print("h ");
  Watchy::display.print(minutes);
  Watchy::display.println("m");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 115);
  Watchy::display.println("Since cold boot");
}

void drawUptime(Watchy &watch) {
  watch.RTC.read(watch.currentTime);
  time_t started = makeTime(bootTime);
  time_t current = makeTime(watch.currentTime);
  drawUptime(current >= started ? current - started : 0);
}

void drawShakeCount(uint16_t count) {
  beginAppDisplay("SHAKE COUNTER");
  useBodyText(48, 90);
  Watchy::display.setTextSize(4);
  Watchy::display.println(count);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 150);
  Watchy::display.println("Shake watch to count");
  Watchy::display.setCursor(10, 180);
  Watchy::display.println("BACK: EXIT");
  finishAppDisplay();
}

void runShakeCounter() {
  WatchyUi::Input::begin();
  Accel previous = {};
  Accel current = {};
  readAcceleration(previous);
  uint16_t count = 0;
  uint32_t lastShake = 0;
  drawShakeCount(count);
  while (true) {
    if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) {
      return;
    }
    if (readAcceleration(current)) {
      float dx = current.x - previous.x;
      float dy = current.y - previous.y;
      float dz = current.z - previous.z;
      float delta = sqrtf(dx * dx + dy * dy + dz * dz) / ACCEL_LSB_PER_G;
      if (delta > 1.15f && millis() - lastShake > 350) {
        lastShake = millis();
        count++;
        Watchy::vibMotor(30, 1);
        drawShakeCount(count);
      }
      previous = current;
    }
    delay(25);
  }
}

} // namespace

void Watchy::showSensorTool(uint8_t tool) {
  if (tool >= SENSOR_TOOL_COUNT) {
    tool = BATTERY_GAUGE;
  }
  if (tool == SHAKE_COUNTER) {
    runShakeCounter();
    showMenu(menuIndex, false);
    return;
  }

  beginAppDisplay(titles[tool]);
  Accel acceleration = {};
  bool accelerationReady = tool < BMA_TEMPERATURE || tool > MOTION_SCORE ||
                           readAcceleration(acceleration);
  if (!accelerationReady) {
    useBodyText();
    display.println("Sensor read failed");
    finishAppDisplay();
    return;
  }

  uint32_t steps = sensor.getCounter();
  switch (tool) {
  case BATTERY_GAUGE: drawBatteryGauge(*this); break;
  case POWER_BUDGET: drawPowerBudget(*this); break;
  case CHARGE_STATUS: drawChargeStatus(*this); break;
  case BMA_TEMPERATURE: drawTemperature(); break;
  case RAW_ACCEL: drawRawAcceleration(acceleration); break;
  case G_FORCE: drawGForce(acceleration); break;
  case SPIRIT_LEVEL: drawLevel(acceleration); break;
  case ORIENTATION: drawOrientation(); break;
  case MOTION_SCORE: drawMotionScore(); break;
  case STEP_COUNTER: drawSteps(steps); break;
  case STEP_GOAL: drawStepGoal(steps); break;
  case WALK_DISTANCE: drawDistance(steps); break;
  case STEP_CALORIES: drawCalories(steps); break;
  case ACTIVITY_STATE: drawActivity(); break;
  case SENSOR_STATUS: drawSensorStatus(); break;
  case UPTIME: drawUptime(*this); break;
  default: break;
  }
  finishAppDisplay();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderSensorPreview(uint8_t tool) {
  if (tool >= SENSOR_TOOL_COUNT) {
    tool = BATTERY_GAUGE;
  }
  if (tool == SHAKE_COUNTER) {
    drawShakeCount(7);
    return;
  }

  constexpr uint32_t steps = 6842;
  const Accel acceleration{184, -92, 1002};
  const WatchyBattery::Estimate battery = WatchyBattery::estimate(3.82f);

  beginAppDisplay(titles[tool]);
  switch (tool) {
  case BATTERY_GAUGE: drawBatteryGauge(battery); break;
  case POWER_BUDGET: drawPowerBudget(battery); break;
  case CHARGE_STATUS: drawChargeStatus(battery, true, true); break;
  case BMA_TEMPERATURE: drawTemperature(28.5f); break;
  case RAW_ACCEL: drawRawAcceleration(acceleration); break;
  case G_FORCE: drawGForce(acceleration); break;
  case SPIRIT_LEVEL: drawLevel(acceleration); break;
  case ORIENTATION: drawOrientation("FACE UP"); break;
  case MOTION_SCORE: drawMotionScore(0.084f); break;
  case STEP_COUNTER: drawSteps(steps); break;
  case STEP_GOAL: drawStepGoal(steps); break;
  case WALK_DISTANCE: drawDistance(steps); break;
  case STEP_CALORIES: drawCalories(steps); break;
  case ACTIVITY_STATE: drawActivity("WALKING"); break;
  case SENSOR_STATUS: drawSensorStatus(true, 0x81, 0x00, 472918); break;
  case UPTIME: drawUptime(2 * SECS_PER_DAY + 4 * SECS_PER_HOUR +
                          18 * SECS_PER_MIN); break;
  default: break;
  }
  finishAppDisplay();
}

} // namespace WatchyDemo
#endif