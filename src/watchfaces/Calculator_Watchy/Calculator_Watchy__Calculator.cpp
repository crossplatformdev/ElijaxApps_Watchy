#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_Calculator_Watchy(Watchy &watchy);

class CalculatorWatchy : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Calculator_Watchy(*this); }
};

static CalculatorWatchy watchy(settings);

void setup() {
  watchy.init();
}

void loop() {}

#endif
