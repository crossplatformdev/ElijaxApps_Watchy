#if defined(WATCHY_STANDALONE_WATCHFACE)

#include "../../watchy/Watchy.h"
#include "../../settings/settings.h"
#include "main.h"

void showWatchFace_Captn_Wednesday(Watchy &watchy) {
  if (watchy.currentTime.Hour == 0 && watchy.currentTime.Minute == 0) {
    sensor.resetStepCounter();
  }

  float batt = (watchy.getBatteryVoltage()-3.3);
  if (batt > 1) { batt = 1; } else if (batt < 0) { batt = 0; }

  drawCaptnWednesday(watchy, batt);
}

class WatchyMoto : public Watchy {
public:
  using Watchy::Watchy;
  void drawWatchFace() override { showWatchFace_Captn_Wednesday(*this); }
};

static WatchyMoto watchy(settings);

void setup() {
  // for Screenshot of watchface output to serial port, reference instructions at
  // https://github.com/sqfmi/Watchy/wiki/Screenshots-of-Watchfaces
  // Serial.begin(115200);
  // while (!Serial); // wait for serial port to connect. Needed for native USB port on Arduino only
  watchy.init();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}

#endif
