#pragma once

#include "../../../src/watchy/Watchy.h"
#include "../include/images.h"
#include "../include/MatCapSource.h"
#include "../include/BlueNoise200.h"

class MetaBallWatchy : public Watchy
{
public:
  MetaBallWatchy(const watchySettings& s);
  void drawWatchFace();

  float getBatteryFill();
};
