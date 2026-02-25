#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Line.cpp"

void showWatchFace_Line(Watchy &watchy);

class LineWatchFace : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Line(*this); }
};

static LineWatchFace watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // Watchy deep sleeps after init()
}

#endif // WATCHY_STANDALONE_WATCHFACE

