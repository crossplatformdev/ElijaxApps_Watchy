#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_TypoStyle(Watchy &watchy);

class TypoStyle : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_TypoStyle(*this); }
};

static TypoStyle watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}
#endif
