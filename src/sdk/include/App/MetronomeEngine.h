#ifndef WATCHY_METRONOME_ENGINE_H
#define WATCHY_METRONOME_ENGINE_H

#include "MetronomeTiming.h"

namespace WatchyMetronome {

enum class PulseStyle : uint8_t {
  Musical,
  Uniform
};

struct Snapshot {
  uint32_t beat;
  uint32_t skippedBeats;
  uint32_t maximumLatenessUs;
  bool running;
};

bool start(uint16_t bpm, uint8_t accentEvery,
           PulseStyle pulseStyle = PulseStyle::Musical);
void stop();
Snapshot snapshot();

} // namespace WatchyMetronome

#endif