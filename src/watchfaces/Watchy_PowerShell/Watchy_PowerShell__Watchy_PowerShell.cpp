#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_PowerShell(Watchy &watchy);

class WatchyPowerShell : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_PowerShell(*this); }
};

static WatchyPowerShell watchy(settings);

void setup(){
  watchy.init();
}

void loop(){}
#endif
