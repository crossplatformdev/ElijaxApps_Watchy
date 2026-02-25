#ifndef WATCHY_SCREEN_CAPTURE_H
#define WATCHY_SCREEN_CAPTURE_H

#include <Arduino.h>

namespace WatchyDemo {
namespace ScreenCapture {

#ifdef WATCHY_DETERMINISTIC_GALLERY

void begin();
void arm(const char *sceneId);
void emit(const uint8_t *bitmap, uint16_t width, uint16_t height);
void error(const char *message, uint16_t expected, uint16_t actual);
void finish(uint16_t expectedFrames);

#else

inline void begin() {}
inline void arm(const char *) {}
inline void emit(const uint8_t *, uint16_t, uint16_t) {}
inline void error(const char *, uint16_t, uint16_t) {}
inline void finish(uint16_t) {}

#endif

} // namespace ScreenCapture
} // namespace WatchyDemo

#endif