#include "../../watchy/Watchy.h"

#ifdef WATCHY_STANDALONE_WATCHFACE

void showWatchFace_BadForEye(Watchy &watchy);

class BadForEye : public Watchy {
public:
  BadForEye() = default;
  void drawWatchFace() override { showWatchFace_BadForEye(*this); }
};

static BadForEye watchy;

void setup(){
  watchy.init();
}

void loop(){}

#endif
