#include "BcgProcessor.h"
#include <math.h>

namespace WatchyBcg {
namespace {

constexpr uint32_t minimumBeatIntervalMs = 125;
constexpr uint32_t maximumBeatIntervalMs = 2500;
constexpr float baselineGravityAlpha = 0.094f;
constexpr float baselineSignalAlpha = 0.46f;
constexpr float baselineEnvelopeAlpha = 0.016f;
constexpr float minimumPeakThreshold = 0.4f;
constexpr float envelopeThresholdMultiplier = 1.8f;

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

void clearIntervals(State &state) {
  state.intervalSampleSum = 0;
  state.intervalCount = 0;
  state.nextIntervalIndex = 0;
}

void appendInterval(State &state, uint16_t interval) {
  if (state.intervalCount == averageBeatCount) {
    state.intervalSampleSum -= state.intervalSamples[state.nextIntervalIndex];
  } else {
    state.intervalCount++;
  }
  state.intervalSamples[state.nextIntervalIndex] = interval;
  state.intervalSampleSum += interval;
  state.nextIntervalIndex = (state.nextIntervalIndex + 1) % averageBeatCount;
}

Result currentResult(const State &state, bool updated, bool beatDetected) {
  Result result{updated, beatDetected, false, 0};
  if (state.intervalCount != 0 && state.intervalSampleSum != 0) {
    uint32_t denominator = state.intervalSampleSum * 1000UL;
    uint32_t numerator = static_cast<uint32_t>(state.sampleRateMilliHz) *
                         60UL * state.intervalCount;
    result.bpm = static_cast<uint16_t>((numerator + denominator / 2) /
                                       denominator);
    if(result.bpm != 0){
      if(result.bpm > 30 && result.bpm < 220){
        result.valid = true;
      } else {
        result.valid = false;
      }
    } else {
      // If bpm is 0, it is not valid
      result.valid = false;
    }
  }
  return result;
}

} // namespace

void reset(State &state) {
  state = {};
  state.envelope = 1.0f;
}

Result process(State &state, const Sample &sample,
               uint16_t sampleRateMilliHz, ResultMode mode) {
  if (sampleRateMilliHz == 0 ||
      baselineSampleRateMilliHz % sampleRateMilliHz != 0) {
    return {false, false, false, 0};
  }
  if (state.sampleRateMilliHz != sampleRateMilliHz) {
    reset(state);
    state.sampleRateMilliHz = sampleRateMilliHz;
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

  float threshold = state.envelope * envelopeThresholdMultiplier;
  if (threshold < minimumPeakThreshold) threshold = minimumPeakThreshold;
  bool localPeak = state.previousFiltered >
                       state.previousPreviousFiltered &&
                   state.previousFiltered >= state.filtered &&
                   state.previousFiltered > threshold;

  bool beatDetected = false;
  bool estimateUpdated = false;
  if (localPeak) {
    uint32_t interval = state.sampleNumber - state.lastBeatSample;
    if (!state.hasBeat ||
        interval >= minimumBeatSamples(sampleRateMilliHz)) {
      if (state.hasBeat) {
        if (interval <= maximumBeatSamples(sampleRateMilliHz)) {
          appendInterval(state, static_cast<uint16_t>(interval));
        } else {
          clearIntervals(state);
        }
        estimateUpdated = true;
      }
      state.lastBeatSample = state.sampleNumber;
      state.hasBeat = true;
      beatDetected = true;
    }
  }

  state.previousPreviousFiltered = state.previousFiltered;
  state.previousFiltered = state.filtered;
  state.sampleNumber++;
  bool signalExpired = state.hasBeat &&
      state.sampleNumber - state.lastBeatSample >
          maximumBeatSamples(sampleRateMilliHz) * 2U;
  if (signalExpired) {
    clearIntervals(state);
    state.hasBeat = false;
    estimateUpdated = true;
  }
  if (mode == ResultMode::Live) {
    return currentResult(state, estimateUpdated, beatDetected);
  }
  state.windowSampleCount++;
  uint16_t windowSamples =
      static_cast<uint32_t>(sampleRateMilliHz) * resultWindowSeconds /
      1000UL;
  bool windowComplete = state.windowSampleCount >= windowSamples;
  if (windowComplete) {
    state.windowSampleCount = 0;
  }
  return currentResult(state, windowComplete, beatDetected);
}

} // namespace WatchyBcg
