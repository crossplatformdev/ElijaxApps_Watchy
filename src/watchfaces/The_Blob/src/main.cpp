#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "../The_Blob.cpp"

void showWatchFace_Blob(Watchy &watchy);

class WatchyBlob : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Blob(*this); }
};

static WatchyBlob watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif // WATCHY_STANDALONE_WATCHFACE