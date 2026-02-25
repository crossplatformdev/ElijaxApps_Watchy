#include "../../watchy/Watchy.h"
#include "../BCD/src/Watchy_Step.h"

void showWatchFace_Steps(Watchy &watchy) {
  WatchyStep face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
