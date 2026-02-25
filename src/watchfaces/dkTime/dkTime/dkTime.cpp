#include "../../../watchy/Watchy.h"
#include "Watchy_dkTime.h"

void showWatchFace_dkTime(Watchy &watchy) {
  WatchyDkTime face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
