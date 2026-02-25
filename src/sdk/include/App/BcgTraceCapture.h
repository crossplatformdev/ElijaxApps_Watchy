#ifndef WATCHY_BCG_TRACE_CAPTURE_H
#define WATCHY_BCG_TRACE_CAPTURE_H

#include <Arduino.h>

namespace WatchyBcgTrace {

#ifdef WATCHY_BCG_TRACE_CAPTURE

void beginAutomatic(uint16_t sampleRateMilliHz);
void append(int16_t x, int16_t y, int16_t z,
			uint16_t sampleRateMilliHz);
bool measurementComplete();
void serviceSerial();

#else

inline void beginAutomatic(uint16_t) {}
inline void append(int16_t, int16_t, int16_t, uint16_t) {}
inline bool measurementComplete() { return false; }
inline void serviceSerial() {}

#endif

} // namespace WatchyBcgTrace

#endif