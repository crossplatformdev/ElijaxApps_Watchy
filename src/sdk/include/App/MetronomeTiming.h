#ifndef WATCHY_METRONOME_TIMING_H
#define WATCHY_METRONOME_TIMING_H

#include <stdint.h>

namespace WatchyMetronome {

constexpr uint16_t minimumBpm = 30;
constexpr uint16_t maximumBpm = 240;
constexpr uint64_t minuteUs = 60000000ULL;

class BeatSchedule {
public:
  void reset(uint16_t bpm, uint64_t firstDeadlineUs) {
    bpm_ = bpm == 0 ? 1 : bpm;
    deadlineUs_ = firstDeadlineUs;
    wholePeriodUs_ = minuteUs / bpm_;
    periodRemainder_ = static_cast<uint16_t>(minuteUs % bpm_);
    accumulatedRemainder_ = 0;
    beat_ = 1;
  }

  uint64_t deadlineUs() const { return deadlineUs_; }
  uint32_t beat() const { return beat_; }

  bool isAccent(uint8_t accentEvery) const {
    return accentEvery > 0 && beat_ % accentEvery == 0;
  }

  void advance() {
    deadlineUs_ += wholePeriodUs_;
    accumulatedRemainder_ += periodRemainder_;
    if (accumulatedRemainder_ >= bpm_) {
      deadlineUs_++;
      accumulatedRemainder_ -= bpm_;
    }
    beat_++;
  }

  uint32_t skipToLatestDue(uint64_t nowUs) {
    uint32_t skipped = 0;
    while (deadlineUs_ <= nowUs) {
      uint64_t nextDeadlineUs = deadlineUs_ + wholePeriodUs_;
      uint16_t nextRemainder = accumulatedRemainder_ + periodRemainder_;
      if (nextRemainder >= bpm_) {
        nextDeadlineUs++;
        nextRemainder -= bpm_;
      }
      if (nextDeadlineUs > nowUs) {
        break;
      }
      deadlineUs_ = nextDeadlineUs;
      accumulatedRemainder_ = nextRemainder;
      beat_++;
      skipped++;
    }
    return skipped;
  }

private:
  uint64_t deadlineUs_ = 0;
  uint32_t wholePeriodUs_ = 0;
  uint32_t beat_ = 1;
  uint16_t bpm_ = 1;
  uint16_t periodRemainder_ = 0;
  uint16_t accumulatedRemainder_ = 0;
};

} // namespace WatchyMetronome

#endif