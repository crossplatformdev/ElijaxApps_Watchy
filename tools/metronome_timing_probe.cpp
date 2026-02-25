#include <algorithm>
#include <cstdint>
#include <iostream>

#include "app/MetronomeTiming.h"

namespace {

constexpr uint32_t simulatedHours = 12;

bool verifyLongRunPhase() {
  constexpr uint64_t startUs = 1234567;
  for (uint16_t bpm = WatchyMetronome::minimumBpm;
       bpm <= WatchyMetronome::maximumBpm; bpm++) {
    WatchyMetronome::BeatSchedule schedule;
    schedule.reset(bpm, startUs);
    uint32_t beats = static_cast<uint32_t>(bpm) * 60 * simulatedHours;
    for (uint32_t index = 0; index < beats; index++) {
      uint64_t expected =
          startUs + static_cast<uint64_t>(index) *
                        WatchyMetronome::minuteUs / bpm;
      if (schedule.deadlineUs() != expected ||
          schedule.beat() != index + 1) {
        return false;
      }
      schedule.advance();
    }
  }
  return true;
}

bool verifyLateWakeSkipsBursts() {
  WatchyMetronome::BeatSchedule schedule;
  constexpr uint64_t startUs = 1000000;
  constexpr uint64_t periodUs = 500000;
  schedule.reset(120, startUs);
  uint64_t resumedAtUs = startUs + 5 * periodUs + 1200;
  uint32_t skipped = schedule.skipToLatestDue(resumedAtUs);
  if (skipped != 5 || schedule.beat() != 6 ||
      schedule.deadlineUs() > resumedAtUs) {
    return false;
  }
  schedule.advance();
  return schedule.beat() == 7 && schedule.deadlineUs() > resumedAtUs;
}

bool verifyAccentGrid() {
  WatchyMetronome::BeatSchedule schedule;
  schedule.reset(120, 0);
  for (uint32_t beat = 1; beat <= 16; beat++) {
    if (schedule.isAccent(4) != (beat % 4 == 0)) {
      return false;
    }
    schedule.advance();
  }
  return true;
}

} // namespace

int main() {
  bool phaseValid = verifyLongRunPhase();
  bool skipValid = verifyLateWakeSkipsBursts();
  bool accentValid = verifyAccentGrid();
  std::cout << "@WATCHY_METRONOME_TIMING bpms="
            << WatchyMetronome::maximumBpm -
                   WatchyMetronome::minimumBpm + 1
            << " hours=" << simulatedHours
            << " phase_error_us=" << (phaseValid ? 0 : 1)
            << " skip=" << (skipValid ? "ok" : "failed")
            << " accent=" << (accentValid ? "ok" : "failed") << '\n';
  return phaseValid && skipValid && accentValid ? 0 : 1;
}