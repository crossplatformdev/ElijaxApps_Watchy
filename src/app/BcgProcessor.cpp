#include "BcgProcessor.h"
#include "WatchyUi.h"
#include <math.h>

namespace WatchyBcg {
namespace {

constexpr uint32_t minimumBeatIntervalMs = 125;
constexpr uint32_t maximumBeatIntervalMs = 2500;
constexpr float baselineGravityAlpha = 0.094f;
constexpr float baselineSignalAlpha = 0.46f;
constexpr float baselineEnvelopeAlpha = 0.016f;

float rateAdjustedAlpha(float baselineAlpha, uint16_t sampleRateMilliHz) {
  uint16_t decimation = baselineSampleRateMilliHz / sampleRateMilliHz;
  float retained = 1.0f - baselineAlpha;
  float adjustedRetained = 1.0f;
  for (uint16_t index = 0; index < decimation; index++) {
    adjustedRetained *= retained;
  }
  return 1.0f - adjustedRetained;
}

uint16_t minimumBeatSamples(uint16_t sampleRateMilliHz) {
  return (static_cast<uint32_t>(sampleRateMilliHz) *
              minimumBeatIntervalMs + 999999UL) /
         1000000UL;
}

uint16_t maximumBeatSamples(uint16_t sampleRateMilliHz) {
  return static_cast<uint32_t>(sampleRateMilliHz) *
         maximumBeatIntervalMs / 1000000UL;
}

Result publish(State &state, uint16_t sampleRateMilliHz) {
  uint16_t maximumInterval = maximumBeatSamples(sampleRateMilliHz);
  bool recentBeat = state.hasBeat &&
                    state.sampleNumber - state.lastBeatSample <=
                        maximumInterval * 2U;
  Result result{true, false, false, 0};
  if (recentBeat && state.intervalCount >= 2) {
    uint32_t averageIntervalSamples =
        (state.intervalSampleSum + state.intervalCount / 2) /
        state.intervalCount;
    if (averageIntervalSamples != 0) {
      uint16_t bpm =
          (static_cast<uint32_t>(sampleRateMilliHz) * 60U +
           averageIntervalSamples * 500UL) /
          (averageIntervalSamples * 1000UL);
      if (bpm >= 40 && bpm <= 180) {
        result.bpm = bpm;
        result.valid = true;
      }
    }
  }
  state.intervalSampleSum = 0;
  state.intervalCount = 0;
  state.windowSampleCount = 0;
  return result;
}

} // namespace

void reset(State &state) {
  state = {};
  state.envelope = 1.0f;
}

Result process(State &state, const Sample &sample,
               uint16_t sampleRateMilliHz) {
  if (sampleRateMilliHz == 0 ||
      baselineSampleRateMilliHz % sampleRateMilliHz != 0) {
    return {false, false, false, 0};
  }

  float x = sample.x;
  float y = sample.y;
  float z = sample.z;
  float magnitude = sqrtf(x * x + y * y + z * z);
  if (state.gravity == 0.0f) state.gravity = magnitude;

  float gravityAlpha = rateAdjustedAlpha(
      baselineGravityAlpha, sampleRateMilliHz);
  float signalAlpha = rateAdjustedAlpha(
      baselineSignalAlpha, sampleRateMilliHz);
  float envelopeAlpha = rateAdjustedAlpha(
      baselineEnvelopeAlpha, sampleRateMilliHz);
  state.gravity += gravityAlpha * (magnitude - state.gravity);
  float highPassed = magnitude - state.gravity;
  state.filtered += signalAlpha * (highPassed - state.filtered);
  state.envelope += envelopeAlpha *
                    (fabsf(state.filtered) - state.envelope);

  float threshold = state.envelope * 2.2f;
  if (threshold < 0.8f) threshold = 0.8f;
  bool localPeak = state.previousFiltered >
                       state.previousPreviousFiltered &&
                   state.previousFiltered >= state.filtered &&
                   state.previousFiltered > threshold;

  bool beatDetected = false;
  if (localPeak) {
    uint32_t interval = state.sampleNumber - state.lastBeatSample;
    if (!state.hasBeat ||
        interval >= minimumBeatSamples(sampleRateMilliHz)) {
      if (state.hasBeat &&
          interval <= maximumBeatSamples(sampleRateMilliHz)) {
        state.intervalSampleSum += interval;
        state.intervalCount++;
      }
      state.lastBeatSample = state.sampleNumber;
      state.hasBeat = true;
      beatDetected = true;
    }
  }

  state.previousPreviousFiltered = state.previousFiltered;
  state.previousFiltered = state.filtered;
  state.sampleNumber++;
  state.windowSampleCount++;
  uint16_t windowSamples =
      static_cast<uint32_t>(sampleRateMilliHz) * resultWindowSeconds /
      1000UL;
  if (state.windowSampleCount >= windowSamples) {
    Result result = publish(state, sampleRateMilliHz);
    result.beatDetected = beatDetected;
    return result;
  }
  return {false, beatDetected, false, 0};
}

} // namespace WatchyBcg
