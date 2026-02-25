#include "../../../watchy/Watchy.h"
#include "../../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_S2Analog(Watchy &watchy);

class WatchyS2Analog : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_S2Analog(*this); }
};

static WatchyS2Analog watchy(settings);

void setup()
{
  watchy.init(); 
}

void loop(){}

#endif
