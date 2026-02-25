#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "../Digdug_Watch.cpp"

void showWatchFace_Digdug_Watch(Watchy &watchy);

class WatchyDigdugWatch : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Digdug_Watch(*this); }
};

static WatchyDigdugWatch watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif // WATCHY_STANDALONE_WATCHFACE
