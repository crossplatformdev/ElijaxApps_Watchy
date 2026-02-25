#ifndef WATCHY_STARRY_HORIZON_H
#define WATCHY_STARRY_HORIZON_H

#include <Watchy.h>

namespace WatchyStarryHorizon {
  void drawWatchFace(Watchy &watch);

  void drawGrid(Watchy &watch);
  void drawStars(Watchy &watch);
  void drawTime(Watchy &watch);
  void drawDate(Watchy &watch);
  void drawCenteredString(Watchy &watch, const String &text, int x, int y,
                          bool drawBackground);
}

#endif