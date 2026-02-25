#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/WatchfaceRegistry.h"

namespace
{
  struct WatchfaceSelectorState
  {
    Watchy *watchy = nullptr;
    uint8_t selected = 0;
    volatile bool exitRequested = false;
    bool dirty = true;
  };

  static WatchfaceSelectorState sWf;

  static void wfBack(Watchy *watchy)
  {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    sWf.exitRequested = true;
  }

  static void wfMenu(Watchy *watchy)
  {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    currentWatchfaceId = sWf.selected;
    sWf.exitRequested = true;
  }

  static void wfUp(Watchy *watchy)
  {
    UiTemplates::waitForAllButtonsReleased(50, 100);                                    
    if (sWf.selected == 0)
    {
      sWf.selected = WatchfaceRegistry::kWatchfaceCount - 1;
    }
    else
    {
      sWf.selected--;
    }
    sWf.dirty = true;
  }

  static void wfDown(Watchy *watchy)
  {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    sWf.selected++;
    if (sWf.selected >= WatchfaceRegistry::kWatchfaceCount)
    {
      sWf.selected = 0;
    }
    sWf.dirty = true;
  }
} // namespace

void Watchy::showWatchfaceSelector()
{
  guiState = APP_STATE;

  sWf.watchy = this;
  sWf.selected = currentWatchfaceId;
  if (sWf.selected >= WatchfaceRegistry::kWatchfaceCount)
  {
    sWf.selected = 0;
  }
  sWf.exitRequested = false;
  sWf.dirty = true;

  UiTemplates::waitForAllButtonsReleased();

  setButtonHandlers(wfBack, wfUp, wfMenu, wfDown);


  UIControlsRowLayout controls[4] = {
      {"BACK", &Watchy::backPressed},
      {"UP", &Watchy::upPressed},
      {"SET", &Watchy::menuPressed},
      {"DOWN", &Watchy::downPressed},
  };
  UIMenuSpec menuSpec;
  menuSpec.x = 0;
  menuSpec.y = MENU_HEIGHT;
  menuSpec.itemHeight = MENU_HEIGHT;
  menuSpec.font = &FreeMonoBold9pt7b;
  menuSpec.items = WatchfaceRegistry::kWatchfaceMenuItems;
  menuSpec.itemCount = WatchfaceRegistry::kWatchfaceCount;
  menuSpec.selectedIndex = sWf.selected;
  // 200px screen with 20px rows and a 20px top margin fits 9 rows.
  // Rendering 10 rows causes drawing beyond the bottom edge.
  const uint8_t visibleRows = 9;
  menuSpec.startIndex = UiTemplates::calcMenuStartIndex(sWf.selected, visibleRows, WatchfaceRegistry::kWatchfaceCount);
  menuSpec.visibleCount = visibleRows;

  UIAppSpec app{};
  app.texts = nullptr;
  app.textCount = 0;
  app.images = nullptr;
  app.imageCount = 0;
  app.menus = &menuSpec;
  app.menuCount = 1;
  app.buttons = nullptr;
  app.buttonCount = 0;
  app.checkboxes = nullptr;
  app.checkboxCount = 0;

  app.controls[0] = controls[0];
  app.controls[1] = controls[1];
  app.controls[2] = controls[2];
  app.controls[3] = controls[3];

  while (!sWf.exitRequested)
  {
    // Pump buttons continuously; redraw only when needed.
    // Important: avoid re-drawing the controls row every iteration (it is
    // expensive on the GFX buffer and can starve background tasks).
    if (UiSDK::buttonPressed(BACK_BTN_PIN)) {
      backPressed();
    }
    if (UiSDK::buttonPressed(UP_BTN_PIN)) {
      upPressed();
    }
    if (UiSDK::buttonPressed(MENU_BTN_PIN)) {
      menuPressed();
    }
    if (UiSDK::buttonPressed(DOWN_BTN_PIN)) {
      downPressed();
    }
    if (sWf.dirty)
    {
      sWf.dirty = false;

      menuSpec.selectedIndex = sWf.selected;
      menuSpec.startIndex = UiTemplates::calcMenuStartIndex(
          sWf.selected,
          visibleRows,
          WatchfaceRegistry::kWatchfaceCount);

      UiSDK::renderApp(*this, app);
      RTC.read(currentTime);
    }

    // Yield to keep WiFi/USB/RTOS tasks alive; prevents watchdog resets.
    delay(20);
  }
  clearButtonHandlers();
  delay(100);
  showMenu(menuIndex);
}
