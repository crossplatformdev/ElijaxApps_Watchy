#include "CityWeather.h"
#include "../../../settings/settings.h"

static CityWeather watchy(settings);
#ifdef WATCHY_STANDALONE_WATCHFACE
void setup(){
  // Serial.begin(115200);
  // while (!Serial);
  watchy.init();
  watchy.showWatchFace(false);
}

void loop(){}
#endif // WATCHY_STANDALONE_WATCHFACE