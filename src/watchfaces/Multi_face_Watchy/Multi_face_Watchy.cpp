#include "main.h"

void showWatchFace_Multi_face_Watchy(Watchy &watchy) {
  WatchyBrainMultiFace face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
