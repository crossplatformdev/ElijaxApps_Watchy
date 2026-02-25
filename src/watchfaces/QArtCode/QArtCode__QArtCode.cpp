#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_QArtCode(Watchy &watchy);

class WatchyQArtCode : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_QArtCode(*this); }
};

static WatchyQArtCode watchy(settings);

void setup(){
  Serial.begin(9600);
  watchy.init();
}

void loop(){}

#endif
