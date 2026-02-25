#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_MacPaint(Watchy &watchy);

class WatchyMacPaint : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_MacPaint(*this); }
};

static WatchyMacPaint watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
