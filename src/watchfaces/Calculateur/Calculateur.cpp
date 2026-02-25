#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "fonts/MadeSunflower39pt7b.h"

void showWatchFace_Calculateur(Watchy &watchy) {
  const uint32_t secsTotal = (uint32_t)watchy.currentTime.Hour * 3600u +
                             (uint32_t)watchy.currentTime.Minute * 60u +
                             (uint32_t)watchy.currentTime.Second;

  uint32_t decimalMinutes = (secsTotal * 1000u + 43200u) / 86400u;
  if (decimalMinutes > 999u) {
    decimalMinutes = 999u;
  }

  String displayTime;
  if (decimalMinutes == 0u) {
    displayTime = "NEW";
  } else {
    displayTime = String(1000u - decimalMinutes);
  }

  const int x = 100;
  const int y = 125;
  int16_t x1, y1;
  uint16_t w, h;

  UiSDK::initScreen(watchy.display);
  UiSDK::setTextColor(watchy.display, UiSDK::getWatchfaceFg(BASE_POLARITY));
  UiSDK::setFont(watchy.display, &MADE_Sunflower_PERSONAL_USE39pt7b);
  UiSDK::setTextWrap(watchy.display, false);
  UiSDK::getTextBounds(watchy.display, displayTime, x, y, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, x - w / 2, y);
  UiSDK::print(watchy.display, displayTime);
}
