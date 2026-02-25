#ifndef WATCHFACE_REGISTRY_H
#define WATCHFACE_REGISTRY_H

#include <Arduino.h>

#include "Watchy_DOS.h"
#include "Watchy_MacPaint.h"
#include "Watchy_Mario.h"
#include "Watchy_Pokemon.h"
#include "Watchy_StarryHorizon.h"
#include "Watchy_Tetris.h"
#include "Watchy_7_SEG.h"

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