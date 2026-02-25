#include "main.h"

void showWatchFace_Brainwork(Watchy &watchy) {
  WatchyBrain face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
