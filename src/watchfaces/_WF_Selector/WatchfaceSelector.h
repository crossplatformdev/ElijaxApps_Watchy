#ifndef WATCHFACE_SELECTOR_H
#define WATCHFACE_SELECTOR_H

#include <Watchy.h>
#include "../7_SEG/Watchy_7_SEG.h"
#include "../DOS/Watchy_DOS.h"
#include "../MacPaint/Watchy_MacPaint.h"
#include "../Mario/Watchy_Mario.h"
#include "../Pokemon/Watchy_Pokemon.h"
#include "../StarryHorizon/Watchy_StarryHorizon.h"
#include "../Tetris/Watchy_Tetris.h"
#include "WatchfaceRegistry.h"

class WatchfaceSelector : public Watchy {
public:
  explicit WatchfaceSelector(const watchySettings &settings);
  void drawWatchFace() override;
  bool updateWatchFaceData() override;

private:
  Watchy7SEG sevenSegment;
  WatchyDOS dos;
  WatchyMacPaint macPaint;
  WatchyMario mario;
  WatchyPokemon pokemon;
  WatchyStarryHorizon starryHorizon;
  WatchyTetris tetris;
};

#endif