#ifndef WATCHY_BCG_PROCESSOR_H
#define WATCHY_BCG_PROCESSOR_H

#include <stdint.h>

namespace WatchyBcg {

constexpr uint16_t baselineSampleRateMilliHz = 25000;
constexpr uint16_t resultWindowSeconds = 15;

struct Sample {
  int16_t x;
  int16_t y;
  int16_t z;
};

struct State {
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
  bool hasBeat;
};

struct Result {
  bool windowComplete;
  bool beatDetected;
  bool valid;
  uint16_t bpm;
};

void reset(State &state);
Result process(State &state, const Sample &sample,
               uint16_t sampleRateMilliHz = baselineSampleRateMilliHz);

} // namespace WatchyBcg

#endif