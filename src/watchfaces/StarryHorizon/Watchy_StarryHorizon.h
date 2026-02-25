#ifndef WATCHY_STARRY_HORIZON_H
#define WATCHY_STARRY_HORIZON_H

#include <Watchy.h>

class WatchyStarryHorizon : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace();

private:
  void drawGrid();
  void drawStars();
  void drawTime();
  void drawDate();
  void drawCenteredString(const String &text, int x, int y, bool drawBackground);
};

#endif