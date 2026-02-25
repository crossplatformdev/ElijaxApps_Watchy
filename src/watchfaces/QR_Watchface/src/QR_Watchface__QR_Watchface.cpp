#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "../QR_Watchface.cpp"

void showWatchFace_QR_Watchface(Watchy &watchy);

class WatchyQR : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_QR_Watchface(*this); }
};

static WatchyQR watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif // WATCHY_STANDALONE_WATCHFACE
