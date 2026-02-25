#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_Beastie(Watchy &watchy);

class Beastie : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Beastie(*this); }
};

static Beastie watchy(settings);

void setup() {
    watchy.init();
}

void loop() {}

#endif
