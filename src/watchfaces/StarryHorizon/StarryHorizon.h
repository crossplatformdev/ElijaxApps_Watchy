#ifndef WATCHY_STARRY_HORIZON_H
#define WATCHY_STARRY_HORIZON_H

#include "../../watchy/Watchy.h"

class WatchyStarryHorizon : public Watchy {
  using Watchy::Watchy;

public:
  void drawWatchFace();
};

#endif