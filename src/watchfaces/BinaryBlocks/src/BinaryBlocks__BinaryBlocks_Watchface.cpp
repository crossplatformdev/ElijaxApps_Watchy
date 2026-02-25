#if defined(WATCHY_STANDALONE_WATCHFACE)
#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"
#include "../BinaryBlocks.cpp"

void showWatchFace_BinaryBlocks(Watchy &watchy);
void handleBinaryBlocksButtonPress(Watchy &watchy);

class WatchyBinaryBlocks : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_BinaryBlocks(*this); }
  void handleButtonPress() override { handleBinaryBlocksButtonPress(*this); }
};

static WatchyBinaryBlocks watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
#endif
