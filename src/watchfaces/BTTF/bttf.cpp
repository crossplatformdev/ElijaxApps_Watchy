#include "../../watchy/Watchy.h"
#include "../BCD/src/Watchy_BTTF.h"

void showWatchFace_BTTF(Watchy &watchy) {
  WatchyBTTF face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
