#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "main.h"

void showWatchFace_Captn_Wednesday(Watchy &watchy) {
  if (watchy.currentTime.Hour == 0 && watchy.currentTime.Minute == 0) {
    sensor.resetStepCounter();
  }

  float batt = (watchy.getBatteryVoltage()-3.3);
  if (batt > 1) { batt = 1; } else if (batt < 0) { batt = 0; }

  drawCaptnWednesday(watchy, batt);
}
