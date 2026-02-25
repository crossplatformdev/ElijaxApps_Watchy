#include "../../watchy/Watchy.h"
#include "../BCD/src/Watchy_LCARS.h"

void showWatchFace_LCARS(Watchy &watchy) {
  WatchyLCARS face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
