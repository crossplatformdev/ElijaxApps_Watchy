#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Analog.cpp"

void showWatchFace_Analog(Watchy &watchy);

class WatchyAnalog : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Analog(*this); }
};

static WatchyAnalog watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif