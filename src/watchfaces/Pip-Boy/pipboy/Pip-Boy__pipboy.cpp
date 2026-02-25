#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "../Pip-Boy.cpp"

void showWatchFace_PipBoy(Watchy &watchy);

class WatchyPipBoy : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_PipBoy(*this); }
};

static WatchyPipBoy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif
