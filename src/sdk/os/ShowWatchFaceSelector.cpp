#include "WatchyUi.h"
#include "Watchy.h"

#include "WatchFaceRegistry.h"

namespace {

void drawWatchfaceSelector(uint8_t selectedId, uint8_t activeId) {
  WatchyUi::ListView::draw("WATCH FACES", watchfaceNames, WATCHFACE_COUNT,
                           selectedId, "SELECT APPLY     BACK EXIT",
                           activeId);
  WatchyUi::Screen::present();
}

void updateWatchfaceSelector(uint8_t previousId, uint8_t selectedId,
                             uint8_t activeId) {
  WatchyUi::ListView::presentSelectionChange(
      "WATCH FACES", watchfaceNames, WATCHFACE_COUNT, selectedId,
      previousId, "SELECT APPLY     BACK EXIT", activeId);
}

} // namespace

void showWatchfaceSelectorImpl(Watchy *watchy) {
  WatchyUi::Input::begin();
  uint8_t selectedId = getSelectedWatchface() % WATCHFACE_COUNT;
  uint8_t activeId = selectedId;
  drawWatchfaceSelector(selectedId, activeId);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
      return;
    }
    if (event == WatchyUi::Event::UP) {
      uint8_t previousId = selectedId;
      selectedId = WatchyUi::ListView::previous(selectedId, WATCHFACE_COUNT);
      updateWatchfaceSelector(previousId, selectedId, activeId);
    } else if (event == WatchyUi::Event::DOWN) {
      uint8_t previousId = selectedId;
      selectedId = WatchyUi::ListView::next(selectedId, WATCHFACE_COUNT);
      updateWatchfaceSelector(previousId, selectedId, activeId);
    } else if (event == WatchyUi::Event::MENU) {
      if (!saveSelectedWatchface(selectedId)) {
        WatchyUi::Feedback::toast("SETTING NOT SAVED");
        if (WatchyUi::deepSleepDelay(700) ==
            WatchyUi::WakeupReason::BACK_PRESSED) {
          if (watchy != nullptr) {
            watchy->showMenu(menuIndex, false);
          } else {
            WatchySdk::showMenu(menuIndex, false);
          }
          return;
        }
        drawWatchfaceSelector(selectedId, activeId);
        continue;
      }
      if (watchy != nullptr) {
        watchy->RTC.read(watchy->currentTime);
        watchy->showWatchFace();
      } else {
        WatchySdk::RTC.read(WatchySdk::currentTime);
        WatchySdk::showWatchFace();
      }
      return;
    }
  }
}

void Watchy::showWatchfaceSelector() {
  showWatchfaceSelectorImpl(this);
}

void WatchySdk::showWatchfaceSelector() {
  showWatchfaceSelectorImpl(nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderWatchfaceSelectorPreview(uint8_t view) {
  (void)view;
  drawWatchfaceSelector(WATCHFACE_DOS, WATCHFACE_7_SEG);
}

} // namespace WatchyDemo
#endif
