#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "../Shadow_Clock.cpp"

void showWatchFace_ShadowClock(Watchy &watchy);

class WatchyShadowClock : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_ShadowClock(*this); }
};

static WatchyShadowClock watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif // WATCHY_STANDALONE_WATCHFACE