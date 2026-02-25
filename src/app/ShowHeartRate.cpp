#include "WatchyUi.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include "AppDisplay.h"
#include "BcgProcessor.h"
#include "BcgTraceCapture.h"
#include "FallDetection.h"
#include "HeartRate.h"
#include "SensorManager.h"
#include "icons.h"
#include "WatchyPowerDiagnostics.h"

#include "Watchy.h"

RTC_DATA_ATTR uint8_t heartRateBpm = 0;
RTC_DATA_ATTR bool heartRateValid = false;
RTC_DATA_ATTR bool heartRateBeatDetected = false;

namespace {

constexpr uint32_t samplePeriodMs =
  1000000UL / WatchyBcg::baselineSampleRateMilliHz;
constexpr int heartCenterX = 51;
constexpr int heartCenterY = 75;
constexpr WatchyUi::Bounds appHeartBounds{36, 63, 32, 22};
constexpr WatchyUi::Bounds appBpmBounds{76, 49, 112, 49};
constexpr WatchyUi::Bounds appStatusBounds{16, 116, 176, 24};
constexpr uint32_t watchfaceStateMagic = 0x48524248UL;
constexpr uint16_t watchfaceSampleRateMilliHz =
  WatchyBcg::baselineSampleRateMilliHz;
constexpr uint16_t watchfaceFifoBytes = 1024;
constexpr uint16_t watchfaceFifoCapacity = 170;
constexpr uint16_t watchfaceFifoWatermarkSamples = 100;
constexpr uint16_t watchfaceFifoHeadroomSamples =
  watchfaceFifoCapacity - watchfaceFifoWatermarkSamples;
static_assert(watchfaceFifoCapacity * BMA4_ACCEL_DATA_LENGTH <=
          watchfaceFifoBytes,
        "Heart-rate read buffer exceeds the BMA423 FIFO");
static_assert(watchfaceFifoHeadroomSamples >= 60,
              "Heart-rate FIFO needs at least 2.4 s service headroom");
constexpr EventBits_t stopRequestedBit = BIT0;
constexpr EventBits_t measurementRunningBit = BIT1;
constexpr EventBits_t measurementFinishedBit = BIT2;

EventGroupHandle_t measurementEvents = nullptr;
SemaphoreHandle_t measurementMutex = nullptr;
StaticEventGroup_t measurementEventsStorage;
StaticSemaphore_t measurementMutexStorage;
TaskHandle_t measurementTask = nullptr;

struct MeasurementTaskParameters {
  uint32_t durationMs;
  HeartRateUpdateCallback updateCallback;
};

MeasurementTaskParameters taskParameters;

struct AppRenderedHeartRateState {
  uint8_t bpm;
  bool initialized;
  bool valid;
  bool heartFilled;
  bool measuring;
};

AppRenderedHeartRateState appRenderedState{};

volatile bool appHeartFilled = false;
volatile bool appHeartMeasuring = false;
volatile bool appHeartUpdatePending = false;

struct WatchfaceHeartRateState {
  uint32_t magic;
  WatchyBcg::State processor;
  bool active;
};

RTC_DATA_ATTR WatchfaceHeartRateState watchfaceState{};

void resetWatchfaceState(bool active) {
  watchfaceState = {};
  watchfaceState.magic = watchfaceStateMagic;
  WatchyBcg::reset(watchfaceState.processor);
  watchfaceState.active = active;
  heartRateBpm = 0;
  heartRateValid = false;
  heartRateBeatDetected = false;
}

bool recoverWatchfaceFifo() {
  bool restored = WatchySensor::setBackgroundMode(
      WatchySensor::Mode::WatchfaceBcg);
  resetWatchfaceState(restored);
  return restored;
}

WatchyBcg::Result processWatchfaceSample(const Accel &acceleration) {
  WatchyBcg::Result result = WatchyBcg::process(
      watchfaceState.processor,
      {acceleration.x, acceleration.y, acceleration.z},
      watchfaceSampleRateMilliHz);
  if (result.windowComplete) {
    heartRateBpm = result.bpm;
    heartRateValid = result.valid;
  }
  return result;
}

void drawAppHeartRate(bool heartFilled, bool measuring) {
  bool firstRender = !appRenderedState.initialized;
  bool heartChanged = firstRender ||
                      appRenderedState.heartFilled != heartFilled;
  bool bpmChanged = firstRender || appRenderedState.valid != heartRateValid ||
                    appRenderedState.bpm != heartRateBpm;
  bool statusChanged = firstRender ||
                       appRenderedState.measuring != measuring;
  if (!heartChanged && !bpmChanged && !statusChanged) {
    WatchyDiagnostics::recordSkippedDisplayUpdate();
    return;
  }

  uint16_t foreground = WatchyUi::Theme::foreground();
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(foreground);
  if (heartChanged) {
    WatchyUi::GrayPaint::fillRect(
        appHeartBounds, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    drawHeartIcon(heartCenterX, heartCenterY, heartFilled, foreground);
    WatchyUi::Screen::invalidate(appHeartBounds);
  }
  if (bpmChanged) {
    WatchyUi::GrayPaint::fillRect(
        appBpmBounds, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
    char bpmText[10];
    if (heartRateValid) {
      snprintf(bpmText, sizeof(bpmText), "%u BPM", heartRateBpm);
    } else {
      snprintf(bpmText, sizeof(bpmText), "-- BPM");
    }
    Watchy::display.setTextSize(1);
    WatchyUi::Canvas::centeredText(appBpmBounds, bpmText, foreground);
    WatchyUi::Screen::invalidate(appBpmBounds);
  }
  if (statusChanged) {
    WatchyUi::GrayPaint::fillRoundRect(
        appStatusBounds, 3,
        WatchyUi::Theme::tone(measuring ? WatchyUi::ToneRole::SurfaceRaised
                                         : WatchyUi::ToneRole::Surface));
    WatchyUi::Canvas::centeredText(
        appStatusBounds, measuring ? "HOLD VERY STILL"
                                   : heartRateValid ? "MEASUREMENT COMPLETE"
                                                    : "READY",
        1, foreground);
    WatchyUi::Screen::invalidate(appStatusBounds);
  }
  WatchyUi::Screen::presentDirty(APP_STATE);
  appRenderedState = {heartRateBpm, true, heartRateValid,
                      heartFilled, measuring};
}

bool ensureSynchronizationObjects() {
  if (measurementEvents == nullptr) {
    measurementEvents = xEventGroupCreateStatic(&measurementEventsStorage);
    if (measurementEvents != nullptr) {
      xEventGroupSetBits(measurementEvents, measurementFinishedBit);
    }
  }
  if (measurementMutex == nullptr) {
    measurementMutex = xSemaphoreCreateMutexStatic(&measurementMutexStorage);
  }
  return measurementEvents != nullptr && measurementMutex != nullptr;
}

bool stopRequested() {
  return (xEventGroupGetBits(measurementEvents) & stopRequestedBit) != 0;
}

void sleepUntilSample(uint32_t now, uint32_t nextSampleAt) {
  uint32_t remainingMs = nextSampleAt - now;
  vTaskDelay(max<TickType_t>(1, pdMS_TO_TICKS(remainingMs)));
}

void notifyUpdate(HeartRateUpdateCallback callback, bool heartFilled,
                  bool measuring) {
  if (callback == nullptr) {
    return;
  }
  callback(heartFilled, measuring);
}

void queueAppHeartRateUpdate(bool heartFilled, bool measuring) {
  appHeartFilled = heartFilled;
  appHeartMeasuring = measuring;
  appHeartUpdatePending = true;
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
  uint32_t startedAt = millis();
  uint32_t nextSampleAt = startedAt;
  WatchyBcg::State processor{};
  WatchyBcg::reset(processor);
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
    if (!WatchySensor::readAcceleration(acceleration)) {
      continue;
    }
    WatchyBcgTrace::append(acceleration.x, acceleration.y,
                           acceleration.z,
                           WatchyBcg::baselineSampleRateMilliHz);
    WatchyBcg::Result result = WatchyBcg::process(
        processor, {acceleration.x, acceleration.y, acceleration.z},
        WatchyBcg::baselineSampleRateMilliHz);
    if (result.windowComplete) {
      heartRateBpm = result.bpm;
      heartRateValid = result.valid;
    }
    if (result.beatDetected) {
      heartFilled = !heartFilled;
    }
    if (result.windowComplete || result.beatDetected) {
      notifyUpdate(updateCallback, heartFilled, true);
    }
  }

  notifyUpdate(updateCallback, heartRateValid, false);
  return heartRateValid;
}

void heartRateTask(void *parameter) {
  MeasurementTaskParameters parameters =
      *static_cast<MeasurementTaskParameters *>(parameter);
  runHeartRateMeasurement(parameters.durationMs, parameters.updateCallback);
  WatchySensor::releaseForeground(
      WatchySensor::Mode::ForegroundHeartRate);
  WatchyDiagnostics::recordHeartRateStackWords(
      uxTaskGetStackHighWaterMark(nullptr));
  xEventGroupClearBits(measurementEvents, measurementRunningBit);
  xEventGroupSetBits(measurementEvents, measurementFinishedBit);
  measurementTask = nullptr;
  vTaskDelete(nullptr);
}

} // namespace

bool measureHeartRateSilently(uint32_t durationMs,
                              HeartRateUpdateCallback updateCallback) {
  if (!ensureSynchronizationObjects()) {
#ifdef WATCHY_BCG_TRACE_CAPTURE
    Serial.println("@WATCHY_BCG_ERROR 1 synchronization");
    Serial.flush();
#endif
    return false;
  }
  if (xSemaphoreTake(measurementMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
#ifdef WATCHY_BCG_TRACE_CAPTURE
    Serial.println("@WATCHY_BCG_ERROR 1 mutex");
    Serial.flush();
#endif
    return false;
  }

  EventBits_t bits = xEventGroupGetBits(measurementEvents);
  if ((bits & measurementRunningBit) != 0) {
    xSemaphoreGive(measurementMutex);
    return false;
  }
  if (!WatchySensor::acquireForeground(
          WatchySensor::Mode::ForegroundHeartRate)) {
#ifdef WATCHY_BCG_TRACE_CAPTURE
    Serial.println("@WATCHY_BCG_ERROR 1 sensor-mode");
    Serial.flush();
#endif
    xSemaphoreGive(measurementMutex);
    return false;
  }

  taskParameters.durationMs = durationMs;
  taskParameters.updateCallback = updateCallback;
  xEventGroupClearBits(measurementEvents,
                       stopRequestedBit | measurementFinishedBit);
  xEventGroupSetBits(measurementEvents, measurementRunningBit);

  BaseType_t result = xTaskCreate(heartRateTask, "heart-rate", 4096,
                                  &taskParameters, 1, &measurementTask);
  if (result != pdPASS) {
    xEventGroupClearBits(measurementEvents, measurementRunningBit);
    xEventGroupSetBits(measurementEvents, measurementFinishedBit);
    WatchySensor::releaseForeground(
        WatchySensor::Mode::ForegroundHeartRate);
  #ifdef WATCHY_BCG_TRACE_CAPTURE
    Serial.println("@WATCHY_BCG_ERROR 1 task-create");
    Serial.flush();
  #endif
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
  TickType_t timeout = max<TickType_t>(1, pdMS_TO_TICKS(timeoutMs));
  EventBits_t bits = xEventGroupWaitBits(
      measurementEvents, measurementFinishedBit, pdFALSE, pdTRUE, timeout);
  return (bits & measurementFinishedBit) != 0;
}

void abortHeartRateMeasurement() {
  TaskHandle_t task = measurementTask;
  measurementTask = nullptr;
  if (task != nullptr) {
    vTaskDelete(task);
  }
  if (measurementEvents != nullptr) {
    xEventGroupClearBits(measurementEvents, measurementRunningBit);
    xEventGroupSetBits(measurementEvents, measurementFinishedBit);
  }
  WatchySensor::releaseForeground(
      WatchySensor::Mode::ForegroundHeartRate);
}

bool isHeartRateMeasurementRunning() {
  return measurementEvents != nullptr &&
         (xEventGroupGetBits(measurementEvents) & measurementRunningBit) != 0;
}

void setWatchfaceHeartRateMonitoring(bool enabled) {
  FallDetection::Status fallStatus = FallDetection::status();
  if (enabled && fallStatus.armed) {
    enabled = false;
  }
  bool stateValid = watchfaceState.magic == watchfaceStateMagic;
  if (!stateValid) {
    resetWatchfaceState(false);
  }
  if (enabled == watchfaceState.active &&
      (enabled || WatchySensor::backgroundMode() !=
                      WatchySensor::Mode::WatchfaceBcg)) {
    return;
  }
  if (!enabled) {
    if (!fallStatus.armed &&
        WatchySensor::backgroundMode() ==
            WatchySensor::Mode::WatchfaceBcg) {
      WatchySensor::setBackgroundMode(WatchySensor::Mode::Baseline);
    }
    resetWatchfaceState(false);
    return;
  }

  resetWatchfaceState(false);
  if (WatchySensor::setBackgroundMode(
          WatchySensor::Mode::WatchfaceBcg)) {
    watchfaceState.active = true;
  } else {
    WatchySensor::setBackgroundMode(WatchySensor::Mode::Baseline);
  }
}

bool serviceWatchfaceHeartRateMonitoring() {
  if (watchfaceState.magic != watchfaceStateMagic ||
      !watchfaceState.active) {
    return false;
  }

  Accel samples[watchfaceFifoCapacity];
  uint16_t sampleCount = 0;
  if (!WatchySensor::readAccelFifo(samples, watchfaceFifoCapacity,
                                   sampleCount)) {
    recoverWatchfaceFifo();
    return true;
  }
  if (sampleCount >= watchfaceFifoCapacity) {
    recoverWatchfaceFifo();
    return true;
  }

  WatchyDiagnostics::recordBcgFifoService(sampleCount);
  uint8_t previousBpm = heartRateBpm;
  bool previousValid = heartRateValid;
  bool beatDetected = false;
  bool averageReady = false;
  for (uint16_t index = 0; index < sampleCount; index++) {
    WatchyBcgTrace::append(samples[index].x, samples[index].y,
                 samples[index].z,
                 watchfaceSampleRateMilliHz);
    WatchyBcg::Result result = processWatchfaceSample(samples[index]);
    averageReady |= result.windowComplete;
    beatDetected |= result.beatDetected;
  }
  bool previousBeatDetected = heartRateBeatDetected;
  heartRateBeatDetected = beatDetected;
  if (!averageReady && previousBeatDetected == heartRateBeatDetected) {
    return false;
  }

  bool visibleChanged = previousBpm != heartRateBpm ||
                        previousValid != heartRateValid;
  bool heartIconChanged = previousBeatDetected != heartRateBeatDetected;
  WatchyDiagnostics::recordBcgResult(visibleChanged || heartIconChanged);
  return visibleChanged || heartIconChanged;
}

bool isWatchfaceHeartRateMonitoringActive() {
  return watchfaceState.magic == watchfaceStateMagic &&
         watchfaceState.active;
}

uint64_t watchfaceHeartRateWakeMask() {
  return isWatchfaceHeartRateMonitoringActive() ? ACC_INT_2_MASK : 0;
}

void showHeartRateImpl(Watchy *watchy) {
  guiState = APP_STATE;
  WatchyUi::Input::begin();
  appRenderedState = {};
  appHeartUpdatePending = false;

  beginAppDisplay("HEART RATE");
  WatchyUi::GrayPaint::fillRoundRect(
      {12, 35, 176, 72}, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(21, 51);
  Watchy::display.println("EXPERIMENTAL BCG");
  WatchyUi::Widget::footer("BACK STOP");
  finishAppDisplay();
  drawAppHeartRate(false, true);

  if (measureHeartRateSilently(0, queueAppHeartRateUpdate)) {
    while (isHeartRateMeasurementRunning()) {
      WatchyBcgTrace::serviceSerial();
#ifdef WATCHY_BCG_TRACE_CAPTURE
      if (WatchyBcgTrace::measurementComplete()) {
        stopHeartRateMeasurement();
      }
#endif
#ifdef WATCHY_BCG_TRACE_CAPTURE
      constexpr uint32_t serialServiceIntervalMs = 100;
#else
      constexpr uint32_t serialServiceIntervalMs = 50;
#endif
      WatchyUi::Event event =
          WatchyUi::Input::waitScheduled(serialServiceIntervalMs);
      if (appHeartUpdatePending) {
        appHeartUpdatePending = false;
        drawAppHeartRate(appHeartFilled, appHeartMeasuring);
      }
      if (event == WatchyUi::Event::BACK) {
        break;
      }
    }
    stopHeartRateMeasurement();
    if (!waitForHeartRateMeasurement(3000)) {
      WatchyDiagnostics::recordWorkerStopTimeout();
      abortHeartRateMeasurement();
    }
  }

  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showHeartRate() { showHeartRateImpl(this); }

void WatchySdk::showHeartRate() { showHeartRateImpl(nullptr); }

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderHeartRatePreview(uint8_t view) {
  uint8_t previousBpm = heartRateBpm;
  bool previousValid = heartRateValid;
  heartRateBpm = view == 2 ? 72 : 0;
  heartRateValid = view == 2;
  appRenderedState = {};

  beginAppDisplay("HEART RATE");
  WatchyUi::GrayPaint::fillRoundRect(
      {12, 35, 176, 72}, 4,
      WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(21, 51);
  Watchy::display.println(view == 3 ? "SIGNAL INVALID - RETRY"
                                    : "EXPERIMENTAL BCG");
  WatchyUi::Widget::footer("BACK STOP");
  drawAppHeartRate(view == 2, view == 1);

  heartRateBpm = previousBpm;
  heartRateValid = previousValid;
}

} // namespace WatchyDemo
#endif
