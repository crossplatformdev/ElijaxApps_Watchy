#include "MetronomeEngine.h"
#include "WatchyUi.h"
#include "Watchy.h"
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "MetronomeTiming.h"
#include "WatchyPowerDiagnostics.h"

namespace WatchyMetronome {
namespace {

constexpr EventBits_t stopRequestedBit = BIT0;
constexpr EventBits_t runningBit = BIT1;
constexpr EventBits_t finishedBit = BIT2;
constexpr uint32_t firstBeatLeadUs = 50000;
constexpr uint32_t preciseWaitWindowUs = 1500;
constexpr uint32_t uniformPulseUs = 30000;
constexpr uint32_t accentPulseUs = 60000;
constexpr uint32_t stopTimeoutMs = 250;
constexpr uint32_t workerStackBytes = 2048;
constexpr UBaseType_t workerPriority = 3;
constexpr BaseType_t workerCore = 0;

struct WorkerParameters {
  uint16_t bpm;
  uint8_t accentEvery;
  PulseStyle pulseStyle;
};

struct SharedState {
  uint32_t beat;
  uint32_t skippedBeats;
  uint32_t maximumLatenessUs;
  bool running;
};

EventGroupHandle_t events = nullptr;
StaticEventGroup_t eventsStorage;
TaskHandle_t workerTask = nullptr;
WorkerParameters workerParameters{};
SharedState sharedState{};
portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;

bool ensureEvents() {
  if (events == nullptr) {
    events = xEventGroupCreateStatic(&eventsStorage);
    if (events != nullptr) {
      xEventGroupSetBits(events, finishedBit);
    }
  }
  return events != nullptr;
}

bool stopRequested() {
  return (xEventGroupGetBits(events) & stopRequestedBit) != 0;
}

bool waitUntil(uint64_t deadlineUs) {
  while (!stopRequested()) {
    int64_t remainingUs =
        static_cast<int64_t>(deadlineUs) - esp_timer_get_time();
    if (remainingUs <= 0) {
      return true;
    }
    if (remainingUs <= preciseWaitWindowUs) {
      delayMicroseconds(static_cast<uint32_t>(remainingUs));
      continue;
    }
    uint32_t sleepMs = static_cast<uint32_t>(
        (remainingUs - preciseWaitWindowUs) / 1000);
    TickType_t ticks = pdMS_TO_TICKS(sleepMs);
    if (ticks == 0) {
      ticks = 1;
    }
    xEventGroupWaitBits(events, stopRequestedBit, pdFALSE, pdTRUE, ticks);
  }
  return false;
}

void publishBeat(uint32_t beat, uint32_t skippedBeats,
                 uint32_t latenessUs) {
  portENTER_CRITICAL(&stateMux);
  sharedState.beat = beat;
  sharedState.skippedBeats += skippedBeats;
  if (latenessUs > sharedState.maximumLatenessUs) {
    sharedState.maximumLatenessUs = latenessUs;
  }
  portEXIT_CRITICAL(&stateMux);
  WatchyDiagnostics::recordMetronomeBeat(latenessUs, skippedBeats);
}

void metronomeTask(void *parameter) {
  WorkerParameters parameters =
      *static_cast<WorkerParameters *>(parameter);
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  digitalWrite(VIB_MOTOR_PIN, LOW);

  BeatSchedule schedule;
  schedule.reset(parameters.bpm,
                 static_cast<uint64_t>(esp_timer_get_time()) +
                     firstBeatLeadUs);
  while (waitUntil(schedule.deadlineUs())) {
    uint64_t nowUs = static_cast<uint64_t>(esp_timer_get_time());
    uint32_t skippedBeats = schedule.skipToLatestDue(nowUs);
    uint64_t scheduledUs = schedule.deadlineUs();
    uint32_t beat = schedule.beat();
    schedule.advance();

    uint64_t startedUs = static_cast<uint64_t>(esp_timer_get_time());
    uint64_t rawLatenessUs = startedUs > scheduledUs
                                 ? startedUs - scheduledUs
                                 : 0;
    uint32_t latenessUs = rawLatenessUs > UINT32_MAX
                              ? UINT32_MAX
                              : static_cast<uint32_t>(rawLatenessUs);
    publishBeat(beat, skippedBeats, latenessUs);
    uint32_t pulseDurationUs = parameters.pulseStyle == PulseStyle::Uniform ||
                                       parameters.accentEvery == 0 ||
                                       beat % parameters.accentEvery != 0
                                   ? uniformPulseUs
                                   : accentPulseUs;
    digitalWrite(VIB_MOTOR_PIN, HIGH);
    delayMicroseconds(pulseDurationUs);
    digitalWrite(VIB_MOTOR_PIN, LOW);
  }

  digitalWrite(VIB_MOTOR_PIN, LOW);
  WatchyDiagnostics::recordMetronomeStackWords(
      uxTaskGetStackHighWaterMark(nullptr));
  portENTER_CRITICAL(&stateMux);
  sharedState.running = false;
  workerTask = nullptr;
  portEXIT_CRITICAL(&stateMux);
  xEventGroupClearBits(events, runningBit);
  xEventGroupSetBits(events, finishedBit);
  vTaskDelete(nullptr);
}

} // namespace

bool start(uint16_t bpm, uint8_t accentEvery, PulseStyle pulseStyle) {
  if (bpm < minimumBpm || bpm > maximumBpm || accentEvery == 0 ||
      !ensureEvents() ||
      (xEventGroupGetBits(events) & runningBit) != 0) {
    return false;
  }

  workerParameters = {bpm, accentEvery, pulseStyle};
  portENTER_CRITICAL(&stateMux);
  sharedState = {0, 0, 0, true};
  portEXIT_CRITICAL(&stateMux);
  xEventGroupClearBits(events, stopRequestedBit | finishedBit);
  xEventGroupSetBits(events, runningBit);

  BaseType_t result = xTaskCreatePinnedToCore(
      metronomeTask, "metronome", workerStackBytes, &workerParameters,
      workerPriority, &workerTask, workerCore);
  if (result != pdPASS) {
    portENTER_CRITICAL(&stateMux);
    sharedState.running = false;
    workerTask = nullptr;
    portEXIT_CRITICAL(&stateMux);
    xEventGroupClearBits(events, runningBit);
    xEventGroupSetBits(events, finishedBit);
  }
  return result == pdPASS;
}

void stop() {
  if (events == nullptr ||
      (xEventGroupGetBits(events) & runningBit) == 0) {
    digitalWrite(VIB_MOTOR_PIN, LOW);
    return;
  }

  xEventGroupSetBits(events, stopRequestedBit);
  EventBits_t bits = xEventGroupWaitBits(
      events, finishedBit, pdFALSE, pdTRUE,
      max<TickType_t>(1, pdMS_TO_TICKS(stopTimeoutMs)));
  if ((bits & finishedBit) == 0) {
    WatchyDiagnostics::recordWorkerStopTimeout();
    TaskHandle_t task = workerTask;
    workerTask = nullptr;
    if (task != nullptr) {
      vTaskDelete(task);
    }
    digitalWrite(VIB_MOTOR_PIN, LOW);
    portENTER_CRITICAL(&stateMux);
    sharedState.running = false;
    portEXIT_CRITICAL(&stateMux);
    xEventGroupClearBits(events, runningBit);
    xEventGroupSetBits(events, finishedBit);
  }
}

Snapshot snapshot() {
  portENTER_CRITICAL(&stateMux);
  Snapshot result{sharedState.beat, sharedState.skippedBeats,
                  sharedState.maximumLatenessUs, sharedState.running};
  portEXIT_CRITICAL(&stateMux);
  return result;
}

} // namespace WatchyMetronome
