#include "../../watchy/Watchy.h"
#include "src/Watchy_BCD.h"

void showWatchFace_BCD(Watchy &watchy) {
  WatchyBCD face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
