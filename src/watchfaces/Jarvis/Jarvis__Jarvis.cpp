#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Jarvis.cpp"

void showWatchFace_Jarvis(Watchy &watchy);

class WatchyJarvisWrapper : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Jarvis(*this); }
};

static WatchyJarvisWrapper watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif
