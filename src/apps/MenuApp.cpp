#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

enum class MenuCategory : uint8_t {
  Time = 0,
  Setup,
  Sensors,
  Network,
  Online,
  Astronomy,
  StyleInfo,
  Count
};

constexpr uint8_t kTopCount = static_cast<uint8_t>(MenuCategory::Count);

UIMenuItemSpec kTopMenuItems[] = {
  {"Time & Alarms"},
  {"System Settings"},
  {"Sensors"},
  {"Network Tools"},
  {"Apps"},
  {"Astronomy"},
  {"Preferences"},
};

UIMenuItemSpec kTimeMenu[] = {
  {"Alarm"},
  {"Timer"},
  {"Chronometer"},
};

UIMenuItemSpec kSetupMenu[] = {
  {"Set up WiFi"},
  {"Set up Time"},
  {"Sync with NTP"},
};

UIMenuItemSpec kSensorsMenu[] = {
  {"Vibrate Motor"},
  {"Show Accelerometer"},
};

UIMenuItemSpec kNetworkMenu[] = {
  {"Ping"},
  {"Traceroute"},
  {"DNS Look-up"},
  {"Whois"},
  {"Port scan"},
  {"HTTP client"},
};

UIMenuItemSpec kOnlineMenu[] = {
  {"Web Search"},
  {"Text browser"},
  {"News"},
  {"Radio"},
  {"Morse Game"},
};

UIMenuItemSpec kAstronomyMenu[] = {
  {"Sun rise/set"},
  {"Moon rise/set"},
  {"Moon phase"},
};

UIMenuItemSpec kStyleMenu[] = {
  {"Watchfaces"},
  {"Theme"},
  {"Update FW"},
  {"About Watchy"},
};

struct MenuGroup {
  const UIMenuItemSpec *items;
  uint8_t length;
};

const MenuGroup kMenuGroups[] = {
  {kTimeMenu, static_cast<uint8_t>(sizeof(kTimeMenu) / sizeof(kTimeMenu[0]))},
  {kSetupMenu, static_cast<uint8_t>(sizeof(kSetupMenu) / sizeof(kSetupMenu[0]))},
  {kSensorsMenu, static_cast<uint8_t>(sizeof(kSensorsMenu) / sizeof(kSensorsMenu[0]))},
  {kNetworkMenu, static_cast<uint8_t>(sizeof(kNetworkMenu) / sizeof(kNetworkMenu[0]))},
  {kOnlineMenu, static_cast<uint8_t>(sizeof(kOnlineMenu) / sizeof(kOnlineMenu[0]))},
  {kAstronomyMenu, static_cast<uint8_t>(sizeof(kAstronomyMenu) / sizeof(kAstronomyMenu[0]))},
  {kStyleMenu, static_cast<uint8_t>(sizeof(kStyleMenu) / sizeof(kStyleMenu[0]))},
};

const MenuGroup *groupFor(uint8_t category) {
  return (category < kTopCount) ? &kMenuGroups[category] : nullptr;
}

uint8_t lengthFor(uint8_t level, uint8_t category) {
  if (level == 0) {
    return kTopCount;
  }
  const MenuGroup *g = groupFor(category);
  return g ? g->length : 0;
}

const UIMenuItemSpec *itemsFor(uint8_t level, uint8_t category) {
  if (level == 0) {
    return kTopMenuItems;
  }
  const MenuGroup *g = groupFor(category);
  return g ? g->items : nullptr;
}

uint8_t clampIndex(uint8_t idx, uint8_t len) {
  if (len == 0) {
    return 0;
  }
  return (idx >= len) ? static_cast<uint8_t>(len - 1) : idx;
}

void renderMenu(Watchy &watchy, byte menuIndex, uint8_t visibleRows) {
  UiTemplates::waitForAllButtonsReleased();
  const uint8_t itemCount = lengthFor(menuLevel, menuCategory);
  const UIMenuItemSpec *items = itemsFor(menuLevel, menuCategory);
  if (items == nullptr || itemCount == 0) {
    return;
  }

  menuIndex = clampIndex(menuIndex, itemCount);

  UiSDK::initScreen(watchy.display);
  UIMenuSpec menuSpec{};
  menuSpec.x             = 0;
  menuSpec.y             = MENU_HEIGHT;
  menuSpec.itemHeight    = MENU_HEIGHT;
  menuSpec.font          = &FreeMonoBold9pt7b;
  menuSpec.items         = items;
  menuSpec.itemCount     = itemCount;
  menuSpec.selectedIndex = menuIndex;
  menuSpec.visibleCount  = visibleRows;
  menuSpec.startIndex    = UiTemplates::calcMenuStartIndex(menuIndex, visibleRows, itemCount);

  UIAppSpec app{};
  app.menus     = &menuSpec;
  app.menuCount = 1;

  UiSDK::renderApp(watchy, app);

  guiState      = MAIN_MENU_STATE;
  alreadyInMenu = true;
}


uint8_t Watchy::activeMenuLength() const {
  return lengthFor(menuLevel, menuCategory);
}

bool Watchy::isInSubMenu() const {
  return menuLevel != 0;
}

void Watchy::enterSubMenu(uint8_t categoryIndex) {
  if (categoryIndex >= kTopCount) {
    return;
  }
  menuLevel = 1;
  menuCategory = categoryIndex;
  menuIndex = 0;
  alreadyInMenu = false;
}

void Watchy::returnToTopMenu() {
  menuLevel = 0;
  menuCategory = 0;
  menuIndex = clampIndex(menuIndex, kTopCount);
  alreadyInMenu = false;
}

void Watchy::launchMenuAction(uint8_t categoryIndex, uint8_t itemIndex) {
  alreadyInMenu = false;
  
  switch (static_cast<MenuCategory>(categoryIndex)) {
    case MenuCategory::Time:
      if (itemIndex == 0) { showAlarm(); }
      else if (itemIndex == 1) { showTimer(); }
      else if (itemIndex == 2) { showChronometer(); }
      break;
    case MenuCategory::Setup:
      if (itemIndex == 0) { setupWifi(); }
      else if (itemIndex == 1) { showSetTime(); }
      else if (itemIndex == 2) { showSyncNTP(); }
      break;
    case MenuCategory::Sensors:
      if (itemIndex == 0) { showBuzz(); }
      else if (itemIndex == 1) { showAccelerometer(); }
      break;
    case MenuCategory::Network:
      if (itemIndex == 0) { showPing(); }
      else if (itemIndex == 1) { showTraceroute(); }
      else if (itemIndex == 2) { showDig(); }
      else if (itemIndex == 3) { showWhois(); }
      else if (itemIndex == 4) { showPortScanner(); }
      else if (itemIndex == 5) { showPostman(); }
      break;
    case MenuCategory::Online:
      if (itemIndex == 0) { showGoogleSearch(); }
      else if (itemIndex == 1) { showTextBrowserHome(); }
      else if (itemIndex == 2) { showNewsReader(); }
      else if (itemIndex == 3) { showRadio(); }
      else if (itemIndex == 4) { showMorseGame(); }
      break;
    case MenuCategory::Astronomy:
      if (itemIndex == 0) { showSunRise(); }
      else if (itemIndex == 1) { showMoonRise(); }
      else if (itemIndex == 2) { showMoonPhase(); }
      break;
    case MenuCategory::StyleInfo:
      if (itemIndex == 0) { showWatchfaceSelector(); }
      else if (itemIndex == 1) { showInvertColors(); }
      else if (itemIndex == 2) { showUpdateFW(); }
      else if (itemIndex == 3) { showAbout(); }
      break;
    default:
      break;
  }
}

void Watchy::showMenu(byte menuIndex){
  const uint8_t visibleRows = 9; // shared layout
  renderMenu(*this, menuIndex, visibleRows);
}

void Watchy::showFastMenu(byte menuIndex) {
  // Same rendering path keeps things simple; still responds quickly
  const uint8_t visibleRows = 9; // shared layout
  renderMenu(*this, menuIndex, visibleRows);
}
