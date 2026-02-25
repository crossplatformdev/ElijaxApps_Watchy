#include <Watchy.h>
#include "sdk/WatchyUi.h"

void Watchy::showBuzz() {
  display.setFullWindow();
  display.fillScreen(DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);;
  display.setCursor(70, 80);
  display.println("Buzz!");
  display.display(true); // partial refresh
  vibMotor();
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderBuzzPreview() {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(WatchyUi::Theme::background());
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setCursor(70, 80);
  Watchy::display.println("Buzz!");
  Watchy::display.display(true);
}

} // namespace WatchyDemo
#endif