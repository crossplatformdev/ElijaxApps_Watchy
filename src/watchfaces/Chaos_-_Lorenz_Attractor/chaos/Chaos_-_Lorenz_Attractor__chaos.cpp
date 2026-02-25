#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "../Chaos_-_Lorenz_Attractor.cpp"

void showWatchFace_ChaosLorenzAttractor(Watchy &watchy);

class WatchyChaosLorenzAttractor : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_ChaosLorenzAttractor(*this); }
};

static WatchyChaosLorenzAttractor watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif