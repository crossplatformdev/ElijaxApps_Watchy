#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "LCARS.cpp"

void showWatchFace_LCARS(Watchy &watchy);

class WatchyLCARSWrapper : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_LCARS(*this); }
};

static WatchyLCARSWrapper watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif
