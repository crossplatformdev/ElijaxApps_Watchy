#include "WatchyPowerDiagnostics.h"

#ifdef WATCHY_POWER_DIAGNOSTICS

#include "config.h"

namespace WatchyDiagnostics {
namespace {

constexpr uint32_t diagnosticsMagic = 0x52455750UL;
constexpr uint8_t heapCheckpointCount =
  static_cast<uint8_t>(HeapCheckpoint::Count);

struct Counters {
  uint32_t magic;
  uint32_t wakes;
  uint32_t timerWakes;
  uint32_t buttonWakes;
  uint32_t sensorWakes;
  uint32_t displayInitializations;
  uint32_t fullRefreshes;
  uint32_t partialRefreshes;
  uint32_t wifiSessions;
  uint32_t bleSessions;
  uint32_t bcgFifoServices;
  uint32_t bcgSamples;
  uint32_t bcgResults;
  uint32_t bcgVisibleChanges;
  uint32_t skippedDisplayUpdates;
  uint32_t workerStopTimeouts;
  uint32_t minimumHeartRateStackWords;
  uint32_t metronomeBeats;
  uint32_t metronomeSkippedBeats;
  uint32_t maximumMetronomeLatenessUs;
  uint32_t minimumMetronomeStackWords;
  uint32_t minimumFreeHeap;
  uint32_t minimumNetworkHeap[heapCheckpointCount];
  uint32_t minimumNetworkLargestBlock[heapCheckpointCount];
  uint32_t maximumNetworkHeapLoss;
  uint64_t awakeMs;
  uint64_t maximumAwakeMs;
  uint64_t lightSleepMs;
  uint64_t dirtyPixels;
  uint64_t wifiMs;
  uint64_t bleMs;
};

RTC_DATA_ATTR Counters counters{};
uint32_t wakeStartedAt = 0;
uint32_t wifiStartedAt = 0;
uint32_t bleStartedAt = 0;
bool wifiActive = false;
bool bleActive = false;
uint32_t networkSessionStartHeap = 0;

void updateMinimumHeap() {
  uint32_t freeHeap = ESP.getFreeHeap();
  if (counters.minimumFreeHeap == 0 || freeHeap < counters.minimumFreeHeap) {
    counters.minimumFreeHeap = freeHeap;
  }
}

void printPreviousCounters() {
  if (counters.wakes == 0) return;
  uint64_t equivalentFullRefreshMilli =
      counters.dirtyPixels * 1000ULL /
      (static_cast<uint64_t>(DISPLAY_WIDTH) * DISPLAY_HEIGHT);
  Serial.begin(115200);
  Serial.printf(
      "@WATCHY_POWER wakes=%lu timer=%lu button=%lu sensor=%lu "
      "awake_ms=%llu display_init=%lu full=%lu partial=%lu "
      "dirty_pixels=%llu equivalent_full_milli=%llu "
      "light_sleep_ms=%llu bcg_services=%lu bcg_samples=%lu "
      "bcg_results=%lu bcg_changes=%lu display_skips=%lu max_awake_ms=%llu "
      "worker_stop_timeouts=%lu heart_stack_free_words=%lu "
      "metronome_beats=%lu metronome_skipped=%lu "
      "metronome_max_late_us=%lu metronome_stack_free_words=%lu "
      "wifi_sessions=%lu wifi_ms=%llu ble_sessions=%lu ble_ms=%llu "
      "min_heap=%lu net_heap=%lu,%lu,%lu,%lu,%lu "
      "net_block=%lu,%lu,%lu,%lu,%lu net_heap_loss=%lu\n",
      static_cast<unsigned long>(counters.wakes),
      static_cast<unsigned long>(counters.timerWakes),
      static_cast<unsigned long>(counters.buttonWakes),
      static_cast<unsigned long>(counters.sensorWakes), counters.awakeMs,
      static_cast<unsigned long>(counters.displayInitializations),
      static_cast<unsigned long>(counters.fullRefreshes),
      static_cast<unsigned long>(counters.partialRefreshes),
      counters.dirtyPixels,
      equivalentFullRefreshMilli,
      counters.lightSleepMs,
      static_cast<unsigned long>(counters.bcgFifoServices),
      static_cast<unsigned long>(counters.bcgSamples),
      static_cast<unsigned long>(counters.bcgResults),
      static_cast<unsigned long>(counters.bcgVisibleChanges),
      static_cast<unsigned long>(counters.skippedDisplayUpdates),
      counters.maximumAwakeMs,
      static_cast<unsigned long>(counters.workerStopTimeouts),
      static_cast<unsigned long>(counters.minimumHeartRateStackWords),
      static_cast<unsigned long>(counters.metronomeBeats),
      static_cast<unsigned long>(counters.metronomeSkippedBeats),
      static_cast<unsigned long>(counters.maximumMetronomeLatenessUs),
      static_cast<unsigned long>(counters.minimumMetronomeStackWords),
      static_cast<unsigned long>(counters.wifiSessions), counters.wifiMs,
      static_cast<unsigned long>(counters.bleSessions), counters.bleMs,
      static_cast<unsigned long>(counters.minimumFreeHeap),
      static_cast<unsigned long>(counters.minimumNetworkHeap[0]),
      static_cast<unsigned long>(counters.minimumNetworkHeap[1]),
      static_cast<unsigned long>(counters.minimumNetworkHeap[2]),
      static_cast<unsigned long>(counters.minimumNetworkHeap[3]),
      static_cast<unsigned long>(counters.minimumNetworkHeap[4]),
      static_cast<unsigned long>(counters.minimumNetworkLargestBlock[0]),
      static_cast<unsigned long>(counters.minimumNetworkLargestBlock[1]),
      static_cast<unsigned long>(counters.minimumNetworkLargestBlock[2]),
      static_cast<unsigned long>(counters.minimumNetworkLargestBlock[3]),
      static_cast<unsigned long>(counters.minimumNetworkLargestBlock[4]),
      static_cast<unsigned long>(counters.maximumNetworkHeapLoss));
  Serial.flush();
}

} // namespace

void beginWake(esp_sleep_wakeup_cause_t cause, uint64_t ext1Bits) {
  if (counters.magic != diagnosticsMagic) {
    counters = {};
    counters.magic = diagnosticsMagic;
  }
  printPreviousCounters();
  wakeStartedAt = millis();
  counters.wakes++;
  if (cause == ESP_SLEEP_WAKEUP_TIMER) counters.timerWakes++;
  if (cause == ESP_SLEEP_WAKEUP_EXT1) {
    if (ext1Bits & BTN_PIN_MASK) counters.buttonWakes++;
    if (ext1Bits & (ACC_INT_MASK | ACC_INT_2_MASK)) counters.sensorWakes++;
  }
  updateMinimumHeap();
}

void endWake() {
  endWifiSession();
  endBleSession();
  uint32_t awakeDuration = millis() - wakeStartedAt;
  counters.awakeMs += awakeDuration;
  if (awakeDuration > counters.maximumAwakeMs) {
    counters.maximumAwakeMs = awakeDuration;
  }
  updateMinimumHeap();
}

void recordDisplayInitialization() {
  counters.displayInitializations++;
}

void recordDisplayRefresh(bool fullRefresh) {
  if (fullRefresh) counters.fullRefreshes++;
  else counters.partialRefreshes++;
}

void recordDirtyRefresh(uint32_t pixels) {
  counters.dirtyPixels += pixels;
}

void recordBcgFifoService(uint16_t samples) {
  counters.bcgFifoServices++;
  counters.bcgSamples += samples;
}

void recordBcgResult(bool visibleChanged) {
  counters.bcgResults++;
  if (visibleChanged) counters.bcgVisibleChanges++;
  else recordSkippedDisplayUpdate();
}

void recordSkippedDisplayUpdate() {
  counters.skippedDisplayUpdates++;
}

void recordWorkerStopTimeout() {
  counters.workerStopTimeouts++;
}

void recordLightSleep(uint32_t durationMs) {
  counters.lightSleepMs += durationMs;
}

void recordHeartRateStackWords(uint32_t unusedWords) {
  if (counters.minimumHeartRateStackWords == 0 ||
      unusedWords < counters.minimumHeartRateStackWords) {
    counters.minimumHeartRateStackWords = unusedWords;
  }
}

void recordMetronomeBeat(uint32_t latenessUs, uint32_t skippedBeats) {
  counters.metronomeBeats++;
  counters.metronomeSkippedBeats += skippedBeats;
  if (latenessUs > counters.maximumMetronomeLatenessUs) {
    counters.maximumMetronomeLatenessUs = latenessUs;
  }
}

void recordMetronomeStackWords(uint32_t unusedWords) {
  if (counters.minimumMetronomeStackWords == 0 ||
      unusedWords < counters.minimumMetronomeStackWords) {
    counters.minimumMetronomeStackWords = unusedWords;
  }
}

void recordHeapCheckpoint(HeapCheckpoint checkpoint) {
  uint8_t index = static_cast<uint8_t>(checkpoint);
  if (index >= heapCheckpointCount) return;
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t largestBlock = ESP.getMaxAllocHeap();
  if (counters.minimumNetworkHeap[index] == 0 ||
      freeHeap < counters.minimumNetworkHeap[index]) {
    counters.minimumNetworkHeap[index] = freeHeap;
  }
  if (counters.minimumNetworkLargestBlock[index] == 0 ||
      largestBlock < counters.minimumNetworkLargestBlock[index]) {
    counters.minimumNetworkLargestBlock[index] = largestBlock;
  }
  updateMinimumHeap();
}

void beginWifiSession() {
  if (wifiActive) return;
  networkSessionStartHeap = ESP.getFreeHeap();
  recordHeapCheckpoint(HeapCheckpoint::BeforeRadio);
  wifiActive = true;
  wifiStartedAt = millis();
  counters.wifiSessions++;
}

void endWifiSession() {
  if (!wifiActive) return;
  recordHeapCheckpoint(HeapCheckpoint::AfterRadio);
  uint32_t finalHeap = ESP.getFreeHeap();
  if (networkSessionStartHeap > finalHeap) {
    uint32_t loss = networkSessionStartHeap - finalHeap;
    if (loss > counters.maximumNetworkHeapLoss) {
      counters.maximumNetworkHeapLoss = loss;
    }
  }
  counters.wifiMs += millis() - wifiStartedAt;
  wifiActive = false;
  networkSessionStartHeap = 0;
}

void beginBleSession() {
  if (bleActive) return;
  bleActive = true;
  bleStartedAt = millis();
  counters.bleSessions++;
}

void endBleSession() {
  if (!bleActive) return;
  counters.bleMs += millis() - bleStartedAt;
  bleActive = false;
}

} // namespace WatchyDiagnostics

#endif