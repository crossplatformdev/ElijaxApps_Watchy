#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Big_Time.cpp"

void showWatchFace_BigTime(Watchy &watchy);

class WatchyBigTime : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_BigTime(*this); }
};

static WatchyBigTime watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif