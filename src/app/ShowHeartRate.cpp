#include <Watchy.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include "AppDisplay.h"
#include "HeartRate.h"
#include "icons.h"
#include "sdk/WatchyUi.h"

RTC_DATA_ATTR uint8_t heartRateBpm = 0;
RTC_DATA_ATTR bool heartRateValid = false;

namespace {

constexpr uint32_t samplePeriodMs = 25;
constexpr uint32_t minimumBeatIntervalMs = 125;
constexpr uint32_t maximumBeatIntervalMs = 2500;
constexpr int heartCenterX = 45;
constexpr int heartCenterY = 91;
constexpr uint32_t watchfaceStateMagic = 0x48524247UL;
constexpr uint16_t watchfaceSampleRateHz = 25;
constexpr uint16_t watchfaceWindowSamples = watchfaceSampleRateHz * 15;
constexpr uint16_t watchfaceFifoBytes = 1024;
constexpr uint16_t watchfaceFifoCapacity = 170;
constexpr uint8_t watchfaceFifoDownsampling = 2;
constexpr uint16_t watchfaceMaximumExpectedSamples =
  watchfaceSampleRateHz * (HEART_RATE_BACKGROUND_WAKE_SECONDS + 1);
constexpr uint16_t watchfaceMinimumBeatSamples =
  (watchfaceSampleRateHz * minimumBeatIntervalMs + 999) / 1000;
constexpr uint16_t watchfaceMaximumBeatSamples =
  watchfaceSampleRateHz * maximumBeatIntervalMs / 1000;
static_assert(watchfaceSampleRateHz * HEART_RATE_BACKGROUND_WAKE_SECONDS *
            BMA4_ACCEL_DATA_LENGTH <=
          watchfaceFifoBytes,
        "Heart-rate samples must fit in the BMA423 FIFO");
static_assert(watchfaceFifoCapacity * BMA4_ACCEL_DATA_LENGTH <=
          watchfaceFifoBytes,
        "Heart-rate read buffer exceeds the BMA423 FIFO");
constexpr EventBits_t stopRequestedBit = BIT0;
constexpr EventBits_t measurementRunningBit = BIT1;
constexpr EventBits_t measurementFinishedBit = BIT2;

EventGroupHandle_t measurementEvents = nullptr;
SemaphoreHandle_t measurementMutex = nullptr;
SemaphoreHandle_t callbackMutex = nullptr;

struct MeasurementTaskParameters {
  uint32_t durationMs;
  HeartRateUpdateCallback updateCallback;
};

MeasurementTaskParameters taskParameters;

struct WatchfaceHeartRateState {
  uint32_t magic;
  uint32_t sampleNumber;
  uint32_t lastBeatSample;
  uint32_t intervalSampleSum;
  uint16_t intervalCount;
  uint16_t windowSampleCount;
  float gravity;
  float filtered;
  float envelope;
  float previousFiltered;
  float previousPreviousFiltered;
  bool active;
  bool hasBeat;
};

RTC_DATA_ATTR WatchfaceHeartRateState watchfaceState{};

void resetWatchfaceState(bool active) {
  watchfaceState = {};
  watchfaceState.magic = watchfaceStateMagic;
  watchfaceState.envelope = 1.0f;
  watchfaceState.active = active;
  heartRateBpm = 0;
  heartRateValid = false;
}

void publishWatchfaceAverage() {
  bool recentBeat = watchfaceState.hasBeat &&
                    watchfaceState.sampleNumber -
                            watchfaceState.lastBeatSample <=
                        watchfaceMaximumBeatSamples * 2;
  heartRateValid = false;
  if (recentBeat && watchfaceState.intervalCount >= 2) {
    uint32_t averageIntervalSamples =
        (watchfaceState.intervalSampleSum +
         watchfaceState.intervalCount / 2) /
        watchfaceState.intervalCount;
    if (averageIntervalSamples != 0) {
      uint16_t bpm =
          (watchfaceSampleRateHz * 60U + averageIntervalSamples / 2) /
          averageIntervalSamples;
      if (bpm >= 40 && bpm <= 180) {
        heartRateBpm = bpm;
        heartRateValid = true;
      }
    }
  }
  watchfaceState.intervalSampleSum = 0;
  watchfaceState.intervalCount = 0;
  watchfaceState.windowSampleCount = 0;
}

bool processWatchfaceSample(const Accel &acceleration) {
  float magnitude = sqrtf((float)acceleration.x * acceleration.x +
                          (float)acceleration.y * acceleration.y +
                          (float)acceleration.z * acceleration.z);
  if (watchfaceState.gravity == 0.0f) {
    watchfaceState.gravity = magnitude;
  }
  watchfaceState.gravity += 0.094f * (magnitude - watchfaceState.gravity);
  float highPassed = magnitude - watchfaceState.gravity;
  watchfaceState.filtered +=
      0.46f * (highPassed - watchfaceState.filtered);
  watchfaceState.envelope +=
      0.016f * (fabsf(watchfaceState.filtered) - watchfaceState.envelope);

  float threshold = max(0.8f, watchfaceState.envelope * 2.2f);
  bool localPeak =
      watchfaceState.previousFiltered >
          watchfaceState.previousPreviousFiltered &&
      watchfaceState.previousFiltered >= watchfaceState.filtered &&
      watchfaceState.previousFiltered > threshold;

  if (localPeak) {
    uint32_t interval = watchfaceState.sampleNumber -
                        watchfaceState.lastBeatSample;
    if (!watchfaceState.hasBeat || interval >= watchfaceMinimumBeatSamples) {
      if (watchfaceState.hasBeat && interval <= watchfaceMaximumBeatSamples) {
        watchfaceState.intervalSampleSum += interval;
        watchfaceState.intervalCount++;
      }
      watchfaceState.lastBeatSample = watchfaceState.sampleNumber;
      watchfaceState.hasBeat = true;
    }
  }

  watchfaceState.previousPreviousFiltered =
      watchfaceState.previousFiltered;
  watchfaceState.previousFiltered = watchfaceState.filtered;
  watchfaceState.sampleNumber++;
  watchfaceState.windowSampleCount++;
  if (watchfaceState.windowSampleCount >= watchfaceWindowSamples) {
    publishWatchfaceAverage();
    return true;
  }
  return false;
}

void drawAppHeartRate(bool heartFilled, bool measuring) {
  Watchy::display.fillRect(0, 48, DISPLAY_WIDTH, 88, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  drawHeartIcon(heartCenterX, heartCenterY, heartFilled, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
  Watchy::display.setCursor(78, 98);
  if (heartRateValid) {
    Watchy::display.print(heartRateBpm);
    Watchy::display.println(" BPM");
  } else {
    Watchy::display.println("-- BPM");
  }
  Watchy::display.setCursor(22, 125);
  Watchy::display.println(measuring ? "Hold very still" : "Ready");
  Watchy::display.displayWindow(0, 48, DISPLAY_WIDTH, 88);
}

bool ensureSynchronizationObjects() {
  if (measurementEvents == nullptr) {
    measurementEvents = xEventGroupCreate();
    if (measurementEvents != nullptr) {
      xEventGroupSetBits(measurementEvents, measurementFinishedBit);
    }
  }
  if (measurementMutex == nullptr) {
    measurementMutex = xSemaphoreCreateMutex();
  }
  if (callbackMutex == nullptr) {
    callbackMutex = xSemaphoreCreateMutex();
  }
  return measurementEvents != nullptr && measurementMutex != nullptr &&
         callbackMutex != nullptr;
}

bool stopRequested() {
  return (xEventGroupGetBits(measurementEvents) & stopRequestedBit) != 0;
}

void sleepUntilSample(uint32_t now, uint32_t nextSampleAt) {
#ifdef ARDUINO_ESP32S3_DEV
  uint32_t remainingMs = nextSampleAt - now;
  if (remainingMs > 1 &&
      esp_sleep_enable_timer_wakeup(
          static_cast<uint64_t>(remainingMs) * 1000ULL) == ESP_OK) {
    esp_light_sleep_start();
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    return;
  }
#endif
  delay(1);
}

void notifyUpdate(HeartRateUpdateCallback callback, bool heartFilled,
                  bool measuring) {
  if (callback == nullptr) {
    return;
  }
  if (xSemaphoreTake(callbackMutex, portMAX_DELAY) == pdTRUE) {
    callback(heartFilled, measuring);
    xSemaphoreGive(callbackMutex);
  }
}

} // namespace

void drawHeartIcon(int16_t centerX, int16_t centerY, bool filled,
                   uint16_t color) {
  if (!filled) {
    return;
  }

  Watchy::display.drawBitmap(centerX - HEART_ICON_WIDTH / 2,
                             centerY - HEART_ICON_HEIGHT / 2,
                             heart, HEART_ICON_WIDTH, HEART_ICON_HEIGHT,
                             color);
}
namespace {

bool runHeartRateMeasurement(uint32_t durationMs,
                             HeartRateUpdateCallback updateCallback) {
  Acfg previousConfig;
  if (!sensor.getAccelConfig(previousConfig)) {
    heartRateValid = false;
    return false;
  }
  Acfg measurementConfig = previousConfig;
  measurementConfig.odr = BMA4_OUTPUT_DATA_RATE_50HZ;
  measurementConfig.range = BMA4_ACCEL_RANGE_2G;
  measurementConfig.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  measurementConfig.perf_mode = BMA4_CONTINUOUS_MODE;
  if (!sensor.setAccelConfig(measurementConfig) || !sensor.enableAccel()) {
    sensor.setAccelConfig(previousConfig);
    heartRateValid = false;
    return false;
  }

  uint32_t startedAt = millis();
  uint32_t nextSampleAt = startedAt;
  uint32_t lastBeatAt = 0;
  uint16_t intervals[5] = {};
  uint8_t intervalCount = 0;
  uint8_t intervalIndex = 0;
  float gravity = 0.0f;
  float filtered = 0.0f;
  float envelope = 1.0f;
  float previousFiltered = 0.0f;
  float previousPreviousFiltered = 0.0f;
  bool heartFilled = false;
  heartRateValid = false;
  notifyUpdate(updateCallback, false, true);

  while ((durationMs == 0 || millis() - startedAt < durationMs) &&
         !stopRequested()) {
    uint32_t now = millis();
    if ((int32_t)(now - nextSampleAt) < 0) {
      sleepUntilSample(now, nextSampleAt);
      continue;
    }
    nextSampleAt += samplePeriodMs;

    Accel acceleration;
    if (!sensor.getAccel(acceleration)) {
      continue;
    }

    float magnitude = sqrtf((float)acceleration.x * acceleration.x +
                            (float)acceleration.y * acceleration.y +
                            (float)acceleration.z * acceleration.z);
    if (gravity == 0.0f) {
      gravity = magnitude;
    }
    gravity += 0.06f * (magnitude - gravity);
    float highPassed = magnitude - gravity;
    filtered += 0.32f * (highPassed - filtered);
    envelope += 0.01f * (fabsf(filtered) - envelope);

    if (heartRateValid && lastBeatAt != 0 &&
        now - lastBeatAt > maximumBeatIntervalMs * 2) {
      heartRateValid = false;
      heartFilled = false;
      if (updateCallback != nullptr) {
        notifyUpdate(updateCallback, false, true);
        nextSampleAt = millis() + samplePeriodMs;
        continue;
      }
    }

    float threshold = envelope * 2.2f;
    if (threshold < 0.8f) {
      threshold = 0.8f;
    }
    bool localPeak = previousFiltered > previousPreviousFiltered &&
                     previousFiltered >= filtered &&
                     previousFiltered > threshold;

    if (localPeak) {
      uint32_t interval = now - lastBeatAt;
      if (lastBeatAt == 0 || interval >= minimumBeatIntervalMs) {
        if (lastBeatAt != 0 && interval <= maximumBeatIntervalMs) {
          intervals[intervalIndex] = interval;
          intervalIndex = (intervalIndex + 1) % 5;
          if (intervalCount < 5) {
            intervalCount++;
          }

          uint32_t intervalSum = 0;
          for (uint8_t index = 0; index < intervalCount; index++) {
            intervalSum += intervals[index];
          }
          uint16_t averageInterval = intervalSum / intervalCount;
          uint16_t bpm = 60000UL / averageInterval;
          if (bpm >= 40 && bpm <= 180) {
            heartRateBpm = bpm;
            heartRateValid = intervalCount >= 2;
          }
        }

        lastBeatAt = now;
        heartFilled = !heartFilled;
        if (updateCallback != nullptr) {
          notifyUpdate(updateCallback, heartFilled, true);
          nextSampleAt = millis() + samplePeriodMs;
        }
      }
    }

    previousPreviousFiltered = previousFiltered;
    previousFiltered = filtered;
  }

  sensor.setAccelConfig(previousConfig);
  notifyUpdate(updateCallback, heartRateValid, false);
  return heartRateValid;
}

void heartRateTask(void *parameter) {
  MeasurementTaskParameters parameters =
      *static_cast<MeasurementTaskParameters *>(parameter);
  runHeartRateMeasurement(parameters.durationMs, parameters.updateCallback);
  xEventGroupClearBits(measurementEvents, measurementRunningBit);
  xEventGroupSetBits(measurementEvents, measurementFinishedBit);
  vTaskDelete(nullptr);
}

} // namespace

bool measureHeartRateSilently(uint32_t durationMs,
                              HeartRateUpdateCallback updateCallback) {
  if (!ensureSynchronizationObjects() ||
      xSemaphoreTake(measurementMutex, portMAX_DELAY) != pdTRUE) {
    return false;
  }

  EventBits_t bits = xEventGroupGetBits(measurementEvents);
  if ((bits & measurementRunningBit) != 0) {
    xSemaphoreGive(measurementMutex);
    return false;
  }

  taskParameters.durationMs = durationMs;
  taskParameters.updateCallback = updateCallback;
  xEventGroupClearBits(measurementEvents,
                       stopRequestedBit | measurementFinishedBit);
  xEventGroupSetBits(measurementEvents, measurementRunningBit);

  BaseType_t result = xTaskCreate(heartRateTask, "heart-rate", 4096,
                                  &taskParameters, 1, nullptr);
  if (result != pdPASS) {
    xEventGroupClearBits(measurementEvents, measurementRunningBit);
    xEventGroupSetBits(measurementEvents, measurementFinishedBit);
  }
  xSemaphoreGive(measurementMutex);
  return result == pdPASS;
}

void stopHeartRateMeasurement() {
  if (measurementEvents != nullptr) {
    xEventGroupSetBits(measurementEvents, stopRequestedBit);
  }
}

bool waitForHeartRateMeasurement(uint32_t timeoutMs) {
  if (measurementEvents == nullptr) {
    return true;
  }
  EventBits_t currentBits = xEventGroupGetBits(measurementEvents);
  if ((currentBits & measurementFinishedBit) != 0) {
    return true;
  }
  TickType_t timeout = timeoutMs == UINT32_MAX
      ? portMAX_DELAY
      : pdMS_TO_TICKS(timeoutMs);
  EventBits_t bits = xEventGroupWaitBits(
      measurementEvents, measurementFinishedBit, pdFALSE, pdTRUE, timeout);
  return (bits & measurementFinishedBit) != 0;
}

bool isHeartRateMeasurementRunning() {
  return measurementEvents != nullptr &&
         (xEventGroupGetBits(measurementEvents) & measurementRunningBit) != 0;
}

void setWatchfaceHeartRateMonitoring(bool enabled) {
  bool stateValid = watchfaceState.magic == watchfaceStateMagic;
  if (!stateValid) {
    resetWatchfaceState(false);
  }
  if (enabled == watchfaceState.active) {
    return;
  }
  if (!enabled) {
    sensor.configureAccelFifo(false);
    resetWatchfaceState(false);
    return;
  }

  resetWatchfaceState(false);
  if (sensor.configureAccelFifo(true, watchfaceFifoDownsampling)) {
    watchfaceState.active = true;
  }
}

bool serviceWatchfaceHeartRateMonitoring() {
  if (watchfaceState.magic != watchfaceStateMagic ||
      !watchfaceState.active) {
    return false;
  }

  Accel samples[watchfaceFifoCapacity];
  uint16_t sampleCount = 0;
  if (!sensor.readAccelFifo(samples, watchfaceFifoCapacity, sampleCount)) {
    sensor.configureAccelFifo(true, watchfaceFifoDownsampling);
    resetWatchfaceState(true);
    return true;
  }
  if (sampleCount > watchfaceMaximumExpectedSamples) {
    resetWatchfaceState(true);
  }

  bool averageReady = false;
  for (uint16_t index = 0; index < sampleCount; index++) {
    averageReady |= processWatchfaceSample(samples[index]);
  }
  return averageReady;
}

bool isWatchfaceHeartRateMonitoringActive() {
  return watchfaceState.magic == watchfaceStateMagic &&
         watchfaceState.active;
}

void Watchy::showHeartRate() {
  guiState = APP_STATE;
  WatchyUi::Input::begin();

  beginAppDisplay("HEART RATE");
  Watchy::display.setCursor(22, 125);
  Watchy::display.println("Hold very still");

  Watchy::display.setCursor(0, 158);
  Watchy::display.println("Experimental BCG");
  finishAppDisplay();
  drawAppHeartRate(false, true);

  if (measureHeartRateSilently(0, drawAppHeartRate)) {
    while (isHeartRateMeasurementRunning()) {
      if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) {
        break;
      }
      delay(10);
    }
    stopHeartRateMeasurement();
    waitForHeartRateMeasurement();
  }

  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderHeartRatePreview() {
  uint8_t previousBpm = heartRateBpm;
  bool previousValid = heartRateValid;
  heartRateBpm = 72;
  heartRateValid = true;

  beginAppDisplay("HEART RATE");
  Watchy::display.setCursor(0, 158);
  Watchy::display.println("Experimental BCG");
  drawAppHeartRate(true, false);

  heartRateBpm = previousBpm;
  heartRateValid = previousValid;
}

} // namespace WatchyDemo
#endif