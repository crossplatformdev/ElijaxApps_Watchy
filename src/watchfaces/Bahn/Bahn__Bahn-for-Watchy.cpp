#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Bahn.cpp"

void showWatchFace_Bahn(Watchy &watchy);

class WatchyBahn : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Bahn(*this); }
};

static WatchyBahn watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif // WATCHY_STANDALONE_WATCHFACE
