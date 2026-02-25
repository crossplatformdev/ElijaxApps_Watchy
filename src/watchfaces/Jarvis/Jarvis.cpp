#include "../../watchy/Watchy.h"
#include "../BCD/src/Watchy_Jarvis.h"

void showWatchFace_Jarvis(Watchy &watchy) {
  WatchyJarvis face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
