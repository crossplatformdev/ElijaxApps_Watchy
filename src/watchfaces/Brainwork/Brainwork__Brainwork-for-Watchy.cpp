#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Brainwork.cpp"

void showWatchFace_Brainwork(Watchy &watchy);

class WatchyBrainwork : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Brainwork(*this); }
};

static WatchyBrainwork watchy(settings);

void setup() {
  watchy.init();
}

void loop() {}
#endif // WATCHY_STANDALONE_WATCHFACE
