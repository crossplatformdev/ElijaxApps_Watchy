#include "UtilityToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include "AppDisplay.h"
#include "UtilitySupport.h"

namespace WatchyUtilityTools {
namespace {
void draw() {
  beginAppDisplay("SCREEN RULER");
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  constexpr float pixelsPerMillimeter = 7.23f;
  Watchy::display.drawLine(0, 65, 199, 65, color);
  useSmallText(2, 48);
  Watchy::display.println("0         10        20   27mm");
  for (uint8_t millimeter = 0; millimeter <= 27; millimeter++) {
    int16_t x = static_cast<int16_t>(millimeter * pixelsPerMillimeter + 0.5f);
    Watchy::display.drawLine(x, 65, x, millimeter % 5 == 0 ? 83 : 75, color);
  }
  Watchy::display.drawLine(25, 105, 25, 199, color);
  for (uint8_t millimeter = 0; millimeter <= 13; millimeter++) {
    int16_t y = 105 + static_cast<int16_t>(millimeter * pixelsPerMillimeter + 0.5f);
    Watchy::display.drawLine(25, y, millimeter % 5 == 0 ? 45 : 35, y, color);
  }
  Watchy::display.setCursor(55, 125);
  Watchy::display.println("Approx. 7.23 px/mm");
  Watchy::display.setCursor(55, 145);
  Watchy::display.println("Verify against a ruler");
  finishAppDisplay();
}
} // namespace
void runScreenRuler() { draw(); }
#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderScreenRulerPreview(uint8_t) { draw(); }
#endif
} // namespace WatchyUtilityTools
