#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_Skully(Watchy &watchy);

class Skully : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Skully(*this); }
};

static Skully watchy(settings);

void setup() {
    watchy.init();
}

void loop() {}

#endif
