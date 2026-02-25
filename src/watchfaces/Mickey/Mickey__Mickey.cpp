#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_Mickey(Watchy &watchy);

class Mickey : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Mickey(*this); }
};

static Mickey watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}
#endif
