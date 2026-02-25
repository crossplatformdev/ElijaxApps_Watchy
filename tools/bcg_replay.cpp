#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "BcgProcessor.h"

namespace {

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}

WatchyBcg::Result injectPeak(WatchyBcg::State &state, uint16_t interval) {
  if (state.hasBeat) state.sampleNumber = state.lastBeatSample + interval;
  state.gravity = 1024.0f;
  state.filtered = 0.0f;
  state.envelope = 1.0f;
  state.previousPreviousFiltered = 0.0f;
  state.previousFiltered = 100.0f;
  return WatchyBcg::process(state, {0, 0, 1024}, 25000,
          WatchyBcg::ResultMode::Live);
}

void testRollingAverage() {
  WatchyBcg::State state{};
  WatchyBcg::reset(state);
  auto result = WatchyBcg::process(state, {0, 0, 1024});
  require(!result.valid && result.bpm == 0, "startup must be invalid");
  result = injectPeak(state, 0);
  require(result.beatDetected && !result.valid, "first beat has no interval");
  result = injectPeak(state, 25);
  require(result.valid && result.bpm == 60 && state.intervalCount == 1,
    "startup must use available intervals without warm-up");
  for (uint16_t interval : {20, 30, 15, 35, 25, 20, 30, 15, 35}) {
    injectPeak(state, interval);
  }
  require(state.intervalCount == 10 && state.intervalSampleSum == 250,
    "ten intervals must be retained");
  result = injectPeak(state, 40);
  require(state.intervalCount == 10 && state.intervalSampleSum == 265 &&
        result.valid && result.bpm == 57 && result.windowComplete,
    "eleventh interval must evict only the oldest interval");
  result = WatchyBcg::process(state, {0, 0, 1024}, 25000,
           WatchyBcg::ResultMode::Live);
  require(result.valid && result.bpm == 57 && !result.beatDetected &&
        !result.windowComplete,
    "every sample must return the retained rolling average");
  state.windowSampleCount = 374;
  result = WatchyBcg::process(state, {0, 0, 1024});
    require(result.valid && result.bpm == 57 && result.windowComplete &&
      state.intervalCount == 10 && state.intervalSampleSum == 265,
    "window publication must not clear beat history");
  result = injectPeak(state, 3);
  require(!result.beatDetected && result.bpm == 57 && state.intervalCount == 10,
    "refractory peaks must not alter the average");
  for (uint16_t index = 0; index < 10; index++) result = injectPeak(state, 25);
  require(result.valid && result.bpm == 60 && state.intervalCount == 10 &&
        state.intervalSampleSum == 250 && state.nextIntervalIndex == 1,
    "a full ring rotation must replace all old intervals without growing");
  result = injectPeak(state, 63);
  require(result.beatDetected && result.windowComplete && !result.valid &&
        state.intervalCount == 0,
    "a long beat gap must restart interval history");
  result = injectPeak(state, 25);
  require(result.valid && result.bpm == 60, "recovery must use fresh intervals");
  state.sampleNumber = state.lastBeatSample + 125;
  state.previousFiltered = 0.0f;
  result = WatchyBcg::process(state, {0, 0, 1024}, 25000,
           WatchyBcg::ResultMode::Live);
  require(!result.valid && result.bpm == 0 && result.windowComplete &&
        !state.hasBeat && state.intervalCount == 0,
    "expired signal must invalidate and clear history");
  state.sampleNumber = UINT32_MAX - 10;
  injectPeak(state, 0);
  result = injectPeak(state, 25);
  require(result.valid && result.bpm == 60,
    "sample counter wrap must preserve interval arithmetic");
  uint32_t previousSample = state.sampleNumber;
  result = WatchyBcg::process(state, {0, 0, 1024}, 0);
  require(!result.valid && state.sampleNumber == previousSample,
    "unsupported rates must not mutate state");
  result = WatchyBcg::process(state, {0, 0, 1024}, 12500);
  require(!result.valid && state.intervalCount == 0 && !state.hasBeat,
    "rate changes must discard incompatible sample intervals");
  std::cout << "Rolling ten-beat checks passed\n";
}

std::vector<std::string> splitCsv(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, ',')) fields.push_back(field);
  return fields;
}

size_t columnIndex(const std::vector<std::string> &header,
                   const char *name) {
  for (size_t index = 0; index < header.size(); index++) {
    if (header[index] == name) return index;
  }
  throw std::runtime_error(std::string("missing CSV column: ") + name);
}

int16_t parseAxis(const std::vector<std::string> &fields, size_t index) {
  if (index >= fields.size()) throw std::runtime_error("short CSV row");
  int value = std::stoi(fields[index]);
  if (value < INT16_MIN || value > INT16_MAX) {
    throw std::runtime_error("axis value outside int16 range");
  }
  return static_cast<int16_t>(value);
}

} // namespace

int main(int argc, char **argv) {
  if (argc == 2 && std::string(argv[1]) == "--self-test") {
    try {
      testRollingAverage();
      return 0;
    } catch (const std::exception &error) {
      std::cerr << error.what() << '\n';
      return 1;
    }
  }
  if (argc != 4) {
    std::cerr << "usage: bcg_replay TRACE.csv INPUT_RATE_MILLIHZ "
                 "TARGET_RATE_MILLIHZ\n";
    return 2;
  }

  try {
    uint32_t inputRate = std::stoul(argv[2]);
    uint32_t targetRate = std::stoul(argv[3]);
    if (inputRate == 0 || targetRate == 0 || targetRate > UINT16_MAX ||
        inputRate % targetRate != 0) {
      throw std::runtime_error(
          "target rate must be an integer divisor of input rate");
    }

    std::ifstream input(argv[1]);
    if (!input) throw std::runtime_error("cannot open trace CSV");
    std::string line;
    if (!std::getline(input, line)) throw std::runtime_error("empty trace CSV");
    std::vector<std::string> header = splitCsv(line);
    size_t xColumn = columnIndex(header, "x");
    size_t yColumn = columnIndex(header, "y");
    size_t zColumn = columnIndex(header, "z");

    WatchyBcg::State state{};
    WatchyBcg::reset(state);
    uint32_t decimation = inputRate / targetRate;
    uint32_t inputSamples = 0;
    uint32_t processedSamples = 0;
    uint32_t windows = 0;
    uint32_t validWindows = 0;
    uint32_t detectedBeats = 0;
    int64_t firstValidMs = -1;

    while (std::getline(input, line)) {
      if (line.empty()) continue;
      std::vector<std::string> fields = splitCsv(line);
      uint32_t sourceIndex = inputSamples++;
      if (sourceIndex % decimation != 0) continue;

      WatchyBcg::Result result = WatchyBcg::process(
          state,
          {parseAxis(fields, xColumn), parseAxis(fields, yColumn),
           parseAxis(fields, zColumn)},
          static_cast<uint16_t>(targetRate));
      processedSamples++;
      if (result.beatDetected) detectedBeats++;
      if (!result.windowComplete) continue;

      windows++;
      if (result.valid) {
        validWindows++;
        if (firstValidMs < 0) {
          firstValidMs = static_cast<uint64_t>(processedSamples) *
                         1000000ULL / targetRate;
        }
      }
      std::cout << "@WATCHY_BCG_WINDOW 1 " << targetRate << ' '
                << processedSamples << ' ' << (result.valid ? 1 : 0) << ' '
                << result.bpm << ' ' << detectedBeats << '\n';
    }

    std::cout << "@WATCHY_BCG_SUMMARY 1 " << targetRate << ' '
              << inputSamples << ' ' << processedSamples << ' ' << windows
              << ' ' << validWindows << ' ' << detectedBeats << ' '
              << firstValidMs << '\n';
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "bcg_replay: " << error.what() << '\n';
    return 1;
  }
}