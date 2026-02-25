#ifndef WATCHFACE_REGISTRY_H
#define WATCHFACE_REGISTRY_H

#include <Arduino.h>

enum WatchfaceId : uint8_t {
  WATCHFACE_7_SEG,
  WATCHFACE_BASIC,
  WATCHFACE_DOS,
  WATCHFACE_MACPAINT,
  WATCHFACE_MARIO,
  WATCHFACE_POKEMON,
  WATCHFACE_STARRY_HORIZON,
  WATCHFACE_TETRIS,
  WATCHFACE_COUNT
};

extern const char *const watchfaceNames[WATCHFACE_COUNT];

uint8_t getSelectedWatchface();
void setSelectedWatchface(uint8_t watchfaceId);
bool saveSelectedWatchface(uint8_t watchfaceId);

#endif