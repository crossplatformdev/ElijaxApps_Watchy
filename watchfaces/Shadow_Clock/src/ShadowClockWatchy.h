#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include "../../../src/watchy/Watchy.h"
#include "Vector3.h"
#include "../include/Normal.h"
#include "../include/Diffuse.h"
#include "../include/Gloss.h"
#include "../include/BlueNoise200.h"
#include "../include/GlossPow.h"

class ShadowClockWatchy : public Watchy
{
  using Watchy::Watchy;
public:
  ShadowClockWatchy();
  void drawWatchFace();
  void drawTime();

  int getBatteryFill(int steps);
};

#endif
