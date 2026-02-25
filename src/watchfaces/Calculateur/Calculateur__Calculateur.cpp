#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Calculateur.cpp"

void showWatchFace_Calculateur(Watchy &watchy);

class WatchyCalculateur : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Calculateur(*this); }
};

static WatchyCalculateur watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif
