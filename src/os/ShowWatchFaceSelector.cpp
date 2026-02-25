#include <Watchy.h>
#include "sdk/WatchyUi.h"
#include "watchfaces/_WF_Selector/WatchfaceRegistry.h"

namespace {

void drawWatchfaceSelector(uint8_t selectedId, uint8_t activeId) {
  WatchyUi::ListView::draw("WATCH FACES", watchfaceNames, WATCHFACE_COUNT,
                           selectedId, "SELECT APPLY     BACK EXIT",
                           activeId);
  WatchyUi::Screen::present();
}

} // namespace

void Watchy::showWatchfaceSelector() {
  WatchyUi::Input::begin();
  uint8_t selectedId = getSelectedWatchface() % WATCHFACE_COUNT;
  uint8_t activeId = selectedId;
  drawWatchfaceSelector(selectedId, activeId);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      showMenu(menuIndex, false);
      return;
    }
    if (event == WatchyUi::Event::UP) {
      selectedId = WatchyUi::ListView::previous(selectedId, WATCHFACE_COUNT);
      drawWatchfaceSelector(selectedId, activeId);
    } else if (event == WatchyUi::Event::DOWN) {
      selectedId = WatchyUi::ListView::next(selectedId, WATCHFACE_COUNT);
      drawWatchfaceSelector(selectedId, activeId);
    } else if (event == WatchyUi::Event::SELECT) {
      if (!saveSelectedWatchface(selectedId)) {
        WatchyUi::Feedback::toast("SETTING NOT SAVED");
        delay(700);
        drawWatchfaceSelector(selectedId, activeId);
        continue;
      }
      RTC.read(currentTime);
      showWatchFace();
      return;
    }
  }
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderWatchfaceSelectorPreview() {
  drawWatchfaceSelector(WATCHFACE_DOS, WATCHFACE_7_SEG);
}

} // namespace WatchyDemo
#endif