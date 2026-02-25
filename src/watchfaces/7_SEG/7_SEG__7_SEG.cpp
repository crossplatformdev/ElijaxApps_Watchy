#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_7SEG(Watchy &watchy);

class Watchy7SEG : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_7SEG(*this); }
};

static Watchy7SEG watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
