#include "../../watchy/Watchy.h"
#include "src/Watchy_GSR.h"

void showWatchFace_WatchyGSR(Watchy &watchy) {
  (void)watchy;
  static WatchyGSR gsr;
  gsr.showWatchFace();
}
