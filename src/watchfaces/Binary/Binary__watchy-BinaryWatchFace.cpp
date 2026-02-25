#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h" //include the Watchy library
#include "../../settings/settings.h"
#include "Binary.cpp"

void showWatchFace_Binary(Watchy &watchy);

class WatchyBinary : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Binary(*this); }
};

static WatchyBinary watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif