#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "Exactly-Words.cpp"

void showWatchFace_Exactly_Words(Watchy &watchy);

class ExactlyWordsWatchFace : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Exactly_Words(*this); }
};

static ExactlyWordsWatchFace watchy(settings);

void setup() {
  watchy.init();
}

void loop() {
  // Watchy deep sleeps after init()
}

#endif // WATCHY_STANDALONE_WATCHFACE

