#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include "circuit_board.h"

namespace {

String toBinaryFixedWidth(uint32_t value, uint8_t width) {
  String out;
  out.reserve(width);
  for (int8_t i = static_cast<int8_t>(width) - 1; i >= 0; --i) {
    out += ((value >> i) & 1U) ? '1' : '0';
  }
  return out;
}

} // namespace

void showWatchFace_Binary(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);

  UIImageSpec bg;
  bg.bitmap = circuit_board;
  bg.x = 0;
  bg.y = 0;
  bg.w = 200;
  bg.h = 200;
  bg.fromProgmem = true;
  bg.fillBackground = false;
  UiSDK::renderImage(watchy.display, bg);

  const String hourBits = toBinaryFixedWidth(static_cast<uint32_t>(watchy.currentTime.Hour), 6);
  const String minuteBits = toBinaryFixedWidth(static_cast<uint32_t>(watchy.currentTime.Minute), 6);

  UITextSpec hourText;
  hourText.x = 35;
  hourText.y = 95;
  hourText.w = 0;
  hourText.h = 0;
  hourText.font = &FreeMonoBold18pt7b;
  hourText.fillBackground = false;
  hourText.invert = true;
  hourText.text = hourBits;
  UiSDK::renderText(watchy.display, hourText);

  UITextSpec minuteText = hourText;
  minuteText.y = 125;
  minuteText.text = minuteBits;
  UiSDK::renderText(watchy.display, minuteText);
}
