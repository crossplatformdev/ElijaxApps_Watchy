#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_7SEGLight(Watchy &watchy);

class Watchy7SEGLight : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_7SEGLight(*this); }
};

static Watchy7SEGLight watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}

#endif
