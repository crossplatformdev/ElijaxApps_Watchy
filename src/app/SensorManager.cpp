#include "SensorManager.h"
#include "WatchyUi.h"
#include "AppDefaults.h"

#ifndef WATCHY_BASELINE_ACCEL_ODR
#define WATCHY_BASELINE_ACCEL_ODR BMA4_OUTPUT_DATA_RATE_50HZ
#endif

namespace WatchySensor {
namespace {

constexpr uint32_t stateMagic = 0x52474d53UL;
constexpr uint16_t bcgFifoWatermarkBytes = 100 * BMA4_ACCEL_DATA_LENGTH;
constexpr uint8_t bcgFifoDownsampling = 2;
constexpr uint16_t fifoReadBlockBytes = 120;
static_assert(FALL_MONITORING_ANY_MOTION_THRESHOLD <= 0x07ff,
              "Fall any-motion threshold exceeds BMA423 range");
static_assert(FALL_MONITORING_ANY_MOTION_DURATION <= 0x1fff,
              "Fall any-motion duration exceeds BMA423 range");

struct RetainedState {
  uint32_t magic;
  Mode background;
  Mode active;
  Mode foreground;
};

RTC_DATA_ATTR RetainedState state{};
bma4_dev driver{};
bool driverReady = false;

uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                      uint16_t length) {
  if (length > UINT8_MAX) return 1;
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) return 1;
  Wire.requestFrom(address, static_cast<uint8_t>(length));
  uint16_t index = 0;
  while (Wire.available() && index < length) {
    data[index++] = Wire.read();
  }
  while (Wire.available()) {
    Wire.read();
  }
  return index == length ? BMA4_OK : 1;
}

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                       uint16_t length) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, length);
  return Wire.endTransmission() == 0 ? BMA4_OK : 1;
}

void delayDriver(uint32_t durationMs) {
  delay(durationMs);
}

bool ensureDriver() {
  if (driverReady) return true;
  driver = {};
  driver.dev_addr = BMA4_I2C_ADDR_PRIMARY;
  driver.interface = BMA4_I2C_INTERFACE;
  driver.bus_read = readRegister;
  driver.bus_write = writeRegister;
  driver.delay = delayDriver;
  driver.read_write_len = 8;
  driver.resolution = 12;
  driver.feature_len = BMA423_FEATURE_SIZE;
  if (bma423_init(&driver) != BMA4_OK) {
    return false;
  }
  uint8_t asicLsb = 0;
  uint8_t asicMsb = 0;
  uint16_t result = bma4_read_regs(BMA4_RESERVED_REG_5B_ADDR, &asicLsb, 1,
                                   &driver);
  result |= bma4_read_regs(BMA4_RESERVED_REG_5C_ADDR, &asicMsb, 1, &driver);
  if (result != BMA4_OK) {
    return false;
  }
  driver.asic_data.asic_lsb = asicLsb & 0x0f;
  driver.asic_data.asic_msb = asicMsb;
  driverReady = true;
  return driverReady;
}

uint16_t configureAccelFifo(bool enabled, uint8_t downsampling) {
  uint16_t result = bma4_set_fifo_config(BMA4_FIFO_HEADER, BMA4_DISABLE,
                                          &driver);
  result |= bma4_set_fifo_down_accel(downsampling, &driver);
  result |= bma4_set_accel_fifo_filter_data(BMA4_ENABLE, &driver);
  result |= bma4_set_fifo_config(BMA4_FIFO_ACCEL,
                                 enabled ? BMA4_ENABLE : BMA4_DISABLE,
                                 &driver);
  result |= bma4_set_command_register(0xB0, &driver);
  return result;
}

uint16_t enableStepCounter() {
  uint16_t result = bma423_step_detector_enable(BMA4_ENABLE, &driver);
  result |= bma423_feature_enable(BMA423_STEP_CNTR, BMA4_ENABLE, &driver);
  result |= bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT,
                                  BMA4_ENABLE, &driver);
  return result;
}

bool isBackgroundMode(Mode mode) {
  return mode == Mode::Baseline || mode == Mode::FallMonitoring ||
         mode == Mode::WatchfaceBcg;
}

bool isForegroundMode(Mode mode) {
  return mode == Mode::ForegroundHeartRate ||
         mode == Mode::ForegroundFall || mode == Mode::LiveAcceleration;
}

void ensureState() {
  if (state.magic != stateMagic) {
    state = {stateMagic, Mode::Baseline, Mode::Baseline, Mode::None};
  }
}

Acfg accelerationConfig(uint8_t odr, uint8_t range, uint8_t performance) {
  Acfg configuration{};
  configuration.odr = odr;
  configuration.range = range;
  configuration.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  configuration.perf_mode = performance;
  return configuration;
}

bma4_int_pin_config interruptPinConfig() {
  bma4_int_pin_config configuration{};
  configuration.edge_ctrl = BMA4_LEVEL_TRIGGER;
#ifdef ARDUINO_ESP32S3_DEV
  configuration.lvl = BMA4_ACTIVE_LOW;
#else
  configuration.lvl = BMA4_ACTIVE_HIGH;
#endif
  configuration.od = BMA4_PUSH_PULL;
  configuration.output_en = BMA4_OUTPUT_ENABLE;
  configuration.input_en = BMA4_INPUT_DISABLE;
  return configuration;
}

bool clearExclusiveFeatures() {
  if (!ensureDriver()) return false;
  uint16_t result = bma4_set_advance_power_save(BMA4_DISABLE, &driver);
  result |= bma423_map_interrupt(BMA4_INTR2_MAP,
                                  BMA423_ANY_NO_MOTION_INT,
                                  BMA4_DISABLE, &driver);
  result |= bma423_map_interrupt(BMA4_INTR2_MAP, BMA4_FIFO_WM_INT,
                                  BMA4_DISABLE, &driver);
  result |= bma423_feature_enable(BMA423_ANY_MOTION, BMA4_DISABLE, &driver);
  result |= bma4_set_fifo_wm(0, &driver);
  result |= configureAccelFifo(false, 0);
  result |= bma4_set_interrupt_mode(BMA4_NON_LATCH_MODE, &driver);
  return result == BMA4_OK;
}

bool configureAcceleration(Acfg configuration, bool lowPower) {
  if (!ensureDriver()) return false;
  uint16_t result = BMA4_OK;
  if (!lowPower) {
    result |= bma4_set_advance_power_save(BMA4_DISABLE, &driver);
  }
  result |= bma4_set_accel_config(&configuration, &driver);
  if (lowPower) {
    result |= bma4_set_advance_power_save(BMA4_ENABLE, &driver);
  }
  result |= bma4_set_accel_enable(BMA4_ENABLE, &driver);
  result |= enableStepCounter();
  return result == BMA4_OK;
}

bool applyBaseline() {
  Acfg configuration = accelerationConfig(
      WATCHY_BASELINE_ACCEL_ODR, BMA4_ACCEL_RANGE_2G,
      BMA4_CIC_AVG_MODE);
  return clearExclusiveFeatures() && configureAcceleration(configuration, true);
}

bool applyFallMonitoring() {
  Acfg configuration = accelerationConfig(
      BMA4_OUTPUT_DATA_RATE_50HZ, BMA4_ACCEL_RANGE_8G,
      BMA4_CIC_AVG_MODE);
  bma4_int_pin_config interruptPin = interruptPinConfig();
    bma423_anymotion_config anyMotion{};
    anyMotion.threshold = FALL_MONITORING_ANY_MOTION_THRESHOLD;
    anyMotion.duration = FALL_MONITORING_ANY_MOTION_DURATION;
    anyMotion.nomotion_sel = 0;
    uint16_t interruptStatus = 0;
    return clearExclusiveFeatures() &&
       configureAcceleration(configuration, true) &&
       bma4_set_int_pin_config(&interruptPin, BMA4_INTR2_MAP, &driver) ==
         BMA4_OK &&
       bma4_set_interrupt_mode(BMA4_LATCH_MODE, &driver) == BMA4_OK &&
       bma423_set_any_motion_config(&anyMotion, &driver) == BMA4_OK &&
       bma423_anymotion_enable_axis(BMA423_ALL_AXIS_EN, &driver) == BMA4_OK &&
       bma423_feature_enable(BMA423_ANY_MOTION, BMA4_ENABLE, &driver) ==
         BMA4_OK &&
       configureAccelFifo(true, 1) == BMA4_OK &&
       bma423_read_int_status(&interruptStatus, &driver) == BMA4_OK &&
       bma423_map_interrupt(BMA4_INTR2_MAP, BMA423_ANY_NO_MOTION_INT,
                  BMA4_ENABLE, &driver) == BMA4_OK;
}

bool applyWatchfaceBcg() {
  Acfg configuration = accelerationConfig(
      BMA4_OUTPUT_DATA_RATE_100HZ, BMA4_ACCEL_RANGE_2G,
      BMA4_CIC_AVG_MODE);
  bma4_int_pin_config interruptPin = interruptPinConfig();
    return clearExclusiveFeatures() &&
      configureAcceleration(configuration, true) &&
      bma4_set_int_pin_config(&interruptPin, BMA4_INTR2_MAP, &driver) ==
          BMA4_OK &&
      bma4_set_fifo_wm(bcgFifoWatermarkBytes, &driver) == BMA4_OK &&
      bma423_map_interrupt(BMA4_INTR2_MAP, BMA4_FIFO_WM_INT,
            BMA4_ENABLE, &driver) == BMA4_OK &&
      configureAccelFifo(true, bcgFifoDownsampling) == BMA4_OK;
}

bool applyForeground(Mode mode) {
  bool lowPower = mode == Mode::LiveAcceleration;
  uint8_t odr = mode == Mode::ForegroundFall
                    ? BMA4_OUTPUT_DATA_RATE_100HZ
                    : BMA4_OUTPUT_DATA_RATE_50HZ;
  uint8_t range = mode == Mode::ForegroundFall
                      ? BMA4_ACCEL_RANGE_8G
                      : BMA4_ACCEL_RANGE_2G;
  Acfg configuration = accelerationConfig(
      odr, range, lowPower ? BMA4_CIC_AVG_MODE : BMA4_CONTINUOUS_MODE);
    return clearExclusiveFeatures() &&
      configureAcceleration(configuration, lowPower);
}

bool applyMode(Mode mode) {
  switch (mode) {
  case Mode::Baseline: return applyBaseline();
  case Mode::FallMonitoring: return applyFallMonitoring();
  case Mode::WatchfaceBcg: return applyWatchfaceBcg();
  case Mode::ForegroundHeartRate:
  case Mode::ForegroundFall:
  case Mode::LiveAcceleration: return applyForeground(mode);
  default: return false;
  }
}

bool applyOrRecover(Mode mode) {
  if (applyMode(mode)) {
    state.active = mode;
    return true;
  }
  applyBaseline();
  state.background = Mode::Baseline;
  state.active = Mode::Baseline;
  state.foreground = Mode::None;
  return false;
}

} // namespace

bool initializeBaseline() {
  state = {stateMagic, Mode::Baseline, Mode::Baseline, Mode::None};
  return ensureDriver() &&
         bma423_select_platform(BMA423_WRIST_CONFIG, &driver) == BMA4_OK &&
         applyOrRecover(Mode::Baseline);
}

bool setBackgroundMode(Mode mode) {
  ensureState();
  if (!isBackgroundMode(mode)) return false;
  state.background = mode;
  if (state.foreground != Mode::None) return true;
  return applyOrRecover(mode);
}

bool reapplyBackgroundMode() {
  ensureState();
  if (state.foreground != Mode::None) return false;
  return applyOrRecover(state.background);
}

bool acquireForeground(Mode mode) {
  ensureState();
  if (!isForegroundMode(mode) || state.foreground != Mode::None) return false;
  state.foreground = mode;
  if (applyMode(mode)) {
    state.active = mode;
    return true;
  }
  state.foreground = Mode::None;
  return applyOrRecover(state.background);
}

bool releaseForeground(Mode mode) {
  ensureState();
  if (state.foreground != mode) return false;
  state.foreground = Mode::None;
  return applyOrRecover(state.background);
}

Mode backgroundMode() {
  ensureState();
  return state.background;
}

Mode activeMode() {
  ensureState();
  return state.active;
}

bool readAccelFifo(Accel *samples, uint16_t capacity,
                   uint16_t &sampleCount) {
  sampleCount = 0;
  if (!ensureDriver() || samples == nullptr || capacity == 0) {
    return false;
  }

  uint16_t remainingBytes = 0;
  if (bma4_get_fifo_length(&remainingBytes, &driver) != BMA4_OK) {
    return false;
  }

  uint8_t fifoData[fifoReadBlockBytes];
  while (remainingBytes >= BMA4_ACCEL_DATA_LENGTH &&
         sampleCount < capacity) {
    uint16_t blockBytes = min<uint16_t>(remainingBytes, fifoReadBlockBytes);
    blockBytes = min<uint16_t>(blockBytes,
                               (capacity - sampleCount) *
                                   BMA4_ACCEL_DATA_LENGTH);
    blockBytes -= blockBytes % BMA4_ACCEL_DATA_LENGTH;
    if (blockBytes == 0) break;

    bma4_fifo_frame frame{};
    frame.data = fifoData;
    frame.length = blockBytes;
    bma4_fifo_frame *previousFrame = driver.fifo;
    driver.fifo = &frame;
    uint16_t result = bma4_read_fifo_data(&driver);
    uint16_t blockSamples = blockBytes / BMA4_ACCEL_DATA_LENGTH;
    if (result == BMA4_OK) {
      result = bma4_extract_accel(samples + sampleCount, &blockSamples,
                                  &driver);
    }
    driver.fifo = previousFrame;
    if (result != BMA4_OK) return false;

    sampleCount += blockSamples;
    remainingBytes -= blockBytes;
  }
  return true;
}

bool readAcceleration(Accel &acceleration) {
  return ::sensor.getAccel(acceleration);
}

uint8_t readDirection() {
  return ::sensor.getDirection();
}

Diagnostics readDiagnostics() {
  return {::sensor.getAccelEnable(), static_cast<uint8_t>(::sensor.getStatus()),
          static_cast<uint8_t>(::sensor.getErrorCode())};
}

bool setActivityEnabled(bool enabled) {
  return ::sensor.enableFeature(BMA423_ACTIVITY, enabled);
}

const char *readActivity() {
  return ::sensor.getActivity();
}

float readTemperature() {
  return ::sensor.readTemperature();
}

bool readInterruptStatus(uint16_t &interruptStatus) {
  interruptStatus = 0;
  return ensureDriver() &&
         bma423_read_int_status(&interruptStatus, &driver) == BMA4_OK;
}

uint32_t stepCount() {
  uint8_t data[4] = {};
  if (readRegister(BMA4_I2C_ADDR_PRIMARY, BMA4_STEP_CNT_OUT_0_ADDR, data,
                   sizeof(data)) != BMA4_OK) {
    return 0;
  }
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

bool resetStepCount() {
  return ensureDriver() &&
         bma423_reset_step_counter(&driver) == BMA4_OK;
}

} // namespace WatchySensor
