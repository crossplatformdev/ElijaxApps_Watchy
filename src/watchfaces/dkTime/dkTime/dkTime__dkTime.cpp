#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "dkTime.cpp"

void showWatchFace_dkTime(Watchy &watchy);

class WatchyDkTimeWrapper : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_dkTime(*this); }
};

static WatchyDkTimeWrapper watchy(settings);

void setup() {
  watchy.init();
}

void loop() {}
#endif