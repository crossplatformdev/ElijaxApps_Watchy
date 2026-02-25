#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "BTTF.cpp"

void showWatchFace_BTTF(Watchy &watchy);

class WatchyBTTFWrapper : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_BTTF(*this); }
};

static WatchyBTTFWrapper watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif
