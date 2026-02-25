#ifndef WATCHY_POWER_DIAGNOSTICS_H
#define WATCHY_POWER_DIAGNOSTICS_H

#include <Arduino.h>
#include <esp_sleep.h>

namespace WatchyDiagnostics {

enum class HeapCheckpoint : uint8_t {
	BeforeRadio,
	Connected,
	Downloaded,
	Parsed,
	AfterRadio,
	Count
};

#ifdef WATCHY_POWER_DIAGNOSTICS

void beginWake(esp_sleep_wakeup_cause_t cause, uint64_t ext1Bits);
void endWake();
void recordDisplayInitialization();
void recordDisplayRefresh(bool fullRefresh);
void recordDirtyRefresh(uint32_t pixels);
void recordBcgFifoService(uint16_t samples);
void recordBcgResult(bool visibleChanged);
void recordSkippedDisplayUpdate();
void recordWorkerStopTimeout();
void recordLightSleep(uint32_t durationMs);
void recordHeartRateStackWords(uint32_t unusedWords);
void recordMetronomeBeat(uint32_t latenessUs, uint32_t skippedBeats);
void recordMetronomeStackWords(uint32_t unusedWords);
void recordHeapCheckpoint(HeapCheckpoint checkpoint);
void beginWifiSession();
void endWifiSession();
void beginBleSession();
void endBleSession();

#else

inline void beginWake(esp_sleep_wakeup_cause_t, uint64_t) {}
inline void endWake() {}
inline void recordDisplayInitialization() {}
inline void recordDisplayRefresh(bool) {}
inline void recordDirtyRefresh(uint32_t) {}
inline void recordBcgFifoService(uint16_t) {}
inline void recordBcgResult(bool) {}
inline void recordSkippedDisplayUpdate() {}
inline void recordWorkerStopTimeout() {}
inline void recordLightSleep(uint32_t) {}
inline void recordHeartRateStackWords(uint32_t) {}
inline void recordMetronomeBeat(uint32_t, uint32_t) {}
inline void recordMetronomeStackWords(uint32_t) {}
inline void recordHeapCheckpoint(HeapCheckpoint) {}
inline void beginWifiSession() {}
inline void endWifiSession() {}
inline void beginBleSession() {}
inline void endBleSession() {}

#endif

} // namespace WatchyDiagnostics

#endif