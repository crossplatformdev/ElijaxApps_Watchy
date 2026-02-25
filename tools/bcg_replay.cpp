#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/BcgProcessor.h"

namespace {

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