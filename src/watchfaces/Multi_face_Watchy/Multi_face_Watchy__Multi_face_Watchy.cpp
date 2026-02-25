#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "main.h"
#include "../../settings/settings.h"

static WatchyBrain watchy(settings);

void setup() {
  //Serial.begin(115200);
  //while (!Serial); // wait for serial port to connect. Needed for native USB port on Arduino only
  watchy.init();
}

void loop() {}

#endif // WATCHY_STANDALONE_WATCHFACE
