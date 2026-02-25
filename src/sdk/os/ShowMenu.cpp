#include "WatchyUi.h"
#include "MenuModel.h"
#include "Watchy.h"

extern RTC_DATA_ATTR bool alreadyInMenu;

namespace {

struct MenuItem {
  const char *name;
  MenuAction action;
};

struct MenuCategory {
  const char *name;
  const MenuItem *items;
  uint8_t itemCount;
};

template <typename Value, size_t Count>
constexpr uint8_t arraySize(const Value (&)[Count]) {
  return static_cast<uint8_t>(Count);
}

const MenuItem clocksSkyItems[] = {
    {"Calendar", MENU_ACTION_MONTH_CALENDAR},
    {"Local + UTC", MENU_ACTION_DUAL_TIME},
    {"World Clocks", MENU_ACTION_WORLD_CLOCKS},
    {"UTC Clock", MENU_ACTION_UTC_CLOCK},
    {"Binary Clock", MENU_ACTION_BINARY_CLOCK},
    {"Time Progress", MENU_ACTION_TIME_PROGRESS},
    {"Internet Beats", MENU_ACTION_INTERNET_BEATS},
    {"Decimal Time", MENU_ACTION_DECIMAL_TIME},
    {"Unix Time", MENU_ACTION_UNIX_TIME},
    {"ISO Week", MENU_ACTION_WEEK_NUMBER},
    {"Day of Year", MENU_ACTION_DAY_OF_YEAR},
    {"Julian Day", MENU_ACTION_JULIAN_DAY},
    {"Sun Rise", MENU_ACTION_SUN_RISE},
    {"Moon Rise", MENU_ACTION_MOON_RISE},
    {"Moon Phase", MENU_ACTION_MOON_PHASE},
    {"Tides", MENU_ACTION_TIDES}};

const MenuItem timersFocusItems[] = {
    {"Daily Alarm", MENU_ACTION_DAILY_ALARM},
    {"Stopwatch", MENU_ACTION_STOPWATCH},
    {"Countdown", MENU_ACTION_COUNTDOWN},
    {"Pomodoro", MENU_ACTION_POMODORO},
    {"Intervals", MENU_ACTION_INTERVAL_TIMER},
    {"Metronome", MENU_ACTION_METRONOME}};

const MenuItem healthWellnessItems[] = {
    {"Heart Rate", MENU_ACTION_HEART_RATE},
    {"Medical ID", MENU_ACTION_MEDICAL_ID},
    {"ICE Contact", MENU_ACTION_ICE_CONTACT},
    {"UN Dog Plate", MENU_ACTION_EMERGENCY_PLATE},
    {"Edit Medical ID", MENU_ACTION_EDIT_MEDICAL_ID},
    {"Blood Type", MENU_ACTION_BLOOD_TYPE},
    {"Allergies", MENU_ACTION_ALLERGIES},
    {"Medications", MENU_ACTION_MEDICATIONS},
    {"Conditions", MENU_ACTION_CONDITIONS},
    {"Medication Alert", MENU_ACTION_MEDICATION_REMINDER},
    {"Hydration Alert", MENU_ACTION_HYDRATION_REMINDER},
    {"Breathing Coach", MENU_ACTION_BREATHING_COACH},
    {"Pain Log", MENU_ACTION_PAIN_LOG},
    {"Symptom Note", MENU_ACTION_SYMPTOM_NOTE},
    {"5-4-3-2-1 Calm", MENU_ACTION_GROUNDING}};

const MenuItem safetyFirstAidItems[] = {
    {"SOS Screen", MENU_ACTION_SOS_SCREEN},
    {"SOS BLE Beacon", MENU_ACTION_SOS_BLE},
    {"Fall Detector", MENU_ACTION_FALL_DETECTOR},
    {"Check-In Timer", MENU_ACTION_CHECK_IN_TIMER},
    {"Body Position", MENU_ACTION_BODY_POSITION},
    {"Saved Location", MENU_ACTION_CONFIGURED_LOCATION},
    {"Emergency Nos.", MENU_ACTION_EMERGENCY_NUMBERS},
    {"CPR Metronome", MENU_ACTION_CPR_METRONOME},
    {"Recovery Pos.", MENU_ACTION_RECOVERY_POSITION},
    {"Stroke FAST", MENU_ACTION_STROKE_FAST},
    {"Choking Response", MENU_ACTION_CHOKING_RESPONSE},
    {"Severe Bleeding", MENU_ACTION_SEVERE_BLEEDING},
    {"Anaphylaxis", MENU_ACTION_ANAPHYLAXIS},
    {"Asthma Attack", MENU_ACTION_ASTHMA_ATTACK},
    {"Opioid Overdose", MENU_ACTION_OPIOID_OVERDOSE},
    {"Seizure Aid", MENU_ACTION_SEIZURE_AID},
    {"Burn First Aid", MENU_ACTION_BURN_FIRST_AID},
    {"Heat Emergency", MENU_ACTION_HEAT_EMERGENCY},
    {"Hypothermia", MENU_ACTION_HYPOTHERMIA},
    {"Poisoning", MENU_ACTION_POISONING}};

const MenuItem sensorsActivityItems[] = {
    {"Battery Gauge", MENU_ACTION_BATTERY_GAUGE},
    {"Charge Status", MENU_ACTION_CHARGE_STATUS},
    {"Power Budget", MENU_ACTION_POWER_BUDGET},
    {"Step Counter", MENU_ACTION_STEP_COUNTER},
    {"Step Goal", MENU_ACTION_STEP_GOAL},
    {"Walk Distance", MENU_ACTION_WALK_DISTANCE},
    {"Step Calories", MENU_ACTION_STEP_CALORIES},
    {"Activity State", MENU_ACTION_ACTIVITY_STATE},
    {"Spirit Level", MENU_ACTION_SPIRIT_LEVEL},
    {"Orientation", MENU_ACTION_ORIENTATION},
    {"Accelerometer", MENU_ACTION_ACCELEROMETER},
    {"Raw Accel", MENU_ACTION_RAW_ACCEL},
    {"G Force", MENU_ACTION_G_FORCE},
    {"Motion Score", MENU_ACTION_MOTION_SCORE},
    {"Shake Counter", MENU_ACTION_SHAKE_COUNTER},
    {"BMA Temperature", MENU_ACTION_BMA_TEMPERATURE},
    {"Sensor Status", MENU_ACTION_SENSOR_STATUS},
    {"Uptime", MENU_ACTION_UPTIME}};

const MenuItem everydayToolItems[] = {
    {"Decision Maker", MENU_ACTION_DECISION_MAKER},
    {"Coin Flip", MENU_ACTION_COIN_FLIP},
    {"D6 Dice", MENU_ACTION_D6_DICE},
    {"D20 Dice", MENU_ACTION_D20_DICE},
    {"Random Number", MENU_ACTION_RANDOM_NUMBER},
    {"Password Gen", MENU_ACTION_PASSWORD_GENERATOR},
    {"UUID Generator", MENU_ACTION_UUID_GENERATOR},
    {"Temperature Conv", MENU_ACTION_TEMPERATURE_CONVERTER},
    {"Length Converter", MENU_ACTION_LENGTH_CONVERTER},
    {"Weight Converter", MENU_ACTION_WEIGHT_CONVERTER},
    {"Pace Converter", MENU_ACTION_PACE_CONVERTER},
    {"Base Converter", MENU_ACTION_BASE_CONVERTER},
    {"Screen Ruler", MENU_ACTION_SCREEN_RULER}};

const MenuItem gameItems[] = {
    {"Snake", MENU_ACTION_SNAKE},
    {"Pong", MENU_ACTION_PONG},
    {"Othello", MENU_ACTION_OTHELLO},
    {"Blackjack", MENU_ACTION_BLACKJACK},
    {"Tic Tac Toe", MENU_ACTION_TIC_TAC_TOE},
    {"Lights Out", MENU_ACTION_LIGHTS_OUT},
    {"Nim", MENU_ACTION_NIM},
    {"Rock Paper Sciss", MENU_ACTION_ROCK_PAPER_SCISSORS},
    {"Higher Lower", MENU_ACTION_HIGHER_LOWER},
    {"Number Guess", MENU_ACTION_NUMBER_GUESS},
    {"Quick Math", MENU_ACTION_QUICK_MATH},
    {"Reaction Test", MENU_ACTION_REACTION_TEST},
    {"Balance", MENU_ACTION_BALANCE_CHALLENGE},
    {"Morse Letter", MENU_ACTION_MORSE_LETTER},
    {"Morse Code", MENU_ACTION_MORSE_CODE}};

const MenuItem networkingItems[] = {
    {"Browser", MENU_ACTION_BROWSER},
    {"DuckDuckGo", MENU_ACTION_DUCKDUCKGO},
    {"RSS Feed", MENU_ACTION_RSS_FEED},
    {"WiFi Survey", MENU_ACTION_WIFI_SURVEY},
    {"Ping", MENU_ACTION_PING},
    {"Traceroute", MENU_ACTION_TRACEROUTE},
    {"DNS Query", MENU_ACTION_DNS_QUERY},
    {"Reverse DNS", MENU_ACTION_REVERSE_DNS},
    {"Port Scanner", MENU_ACTION_PORT_SCANNER}};

const MenuItem bluetoothItems[] = {
    {"BLE Scanner", MENU_ACTION_BLE_SCANNER},
    {"Strongest Signal", MENU_ACTION_BLE_STRONGEST},
    {"Device Count", MENU_ACTION_BLE_DEVICE_COUNT},
    {"Named Devices", MENU_ACTION_BLE_NAMED},
    {"BLE Radar", MENU_ACTION_BLE_RADAR},
    {"iBeacon Watch", MENU_ACTION_BLE_IBEACONS},
    {"Service UUIDs", MENU_ACTION_BLE_SERVICES},
    {"Manufacturer IDs", MENU_ACTION_BLE_MANUFACTURERS},
    {"BLE Addresses", MENU_ACTION_BLE_ADDRESSES},
    {"RSSI Bands", MENU_ACTION_BLE_RSSI_BANDS},
    {"TX Power Survey", MENU_ACTION_BLE_TX_POWER},
    {"Watchy Beacon", MENU_ACTION_BLE_BEACON},
    {"Name Badge", MENU_ACTION_BLE_NAME_BADGE},
    {"Battery Beacon", MENU_ACTION_BLE_BATTERY_BEACON},
    {"Time Beacon", MENU_ACTION_BLE_TIME_BEACON},
    {"Step Beacon", MENU_ACTION_BLE_STEP_BEACON}};

const MenuItem watchSystemItems[] = {
    {"Watch Faces", MENU_ACTION_WATCH_FACES},
    {"Theme Colours", MENU_ACTION_THEME_COLOURS},
    {"Set Time", MENU_ACTION_SET_TIME},
    {"Sync NTP", MENU_ACTION_SYNC_NTP},
    {"Setup WiFi", MENU_ACTION_SETUP_WIFI},
    {"Vibrate Motor", MENU_ACTION_VIBRATE},
    {"Vibration Lab", MENU_ACTION_VIBRATION_LAB},
    {"Button Tester", MENU_ACTION_BUTTON_TESTER},
    {"I2C Scanner", MENU_ACTION_I2C_SCANNER},
    {"Chip Info", MENU_ACTION_CHIP_INFO},
    {"Heap Monitor", MENU_ACTION_HEAP_MONITOR},
    {"Wake Reason", MENU_ACTION_WAKE_REASON},
    {"Reset Reason", MENU_ACTION_RESET_REASON},
    {"About Watchy", MENU_ACTION_ABOUT}};

const MenuCategory categories[] = {
    {"Clocks & Sky", clocksSkyItems, arraySize(clocksSkyItems)},
    {"Timers & Focus", timersFocusItems, arraySize(timersFocusItems)},
  {"Health & Care", healthWellnessItems, arraySize(healthWellnessItems)},
  {"Safety & Aid", safetyFirstAidItems, arraySize(safetyFirstAidItems)},
  {"Sensors & Steps", sensorsActivityItems,
   arraySize(sensorsActivityItems)},
    {"Everyday Tools", everydayToolItems, arraySize(everydayToolItems)},
    {"Games & Puzzles", gameItems, arraySize(gameItems)},
    {"Network & Web", networkingItems, arraySize(networkingItems)},
    {"Bluetooth", bluetoothItems, arraySize(bluetoothItems)},
    {"Watch & System", watchSystemItems, arraySize(watchSystemItems)}};

constexpr uint16_t applicationCount =
    arraySize(clocksSkyItems) + arraySize(timersFocusItems) +
    arraySize(healthWellnessItems) + arraySize(safetyFirstAidItems) +
    arraySize(sensorsActivityItems) + arraySize(everydayToolItems) +
    arraySize(gameItems) + arraySize(networkingItems) +
    arraySize(bluetoothItems) + arraySize(watchSystemItems);
static_assert(applicationCount == MENU_ACTION_BALANCE_CHALLENGE + 1,
              "Menu must contain all registered applications");

constexpr int categoryCount = arraySize(categories);
static_assert(categoryCount == MENU_CATEGORY_COUNT,
              "MENU_CATEGORY_COUNT must match categories");

struct MenuListContext {
  bool showingCategories;
  uint8_t category;
};

const char *menuLabel(uint8_t index, const void *rawContext) {
  const MenuListContext &context =
      *static_cast<const MenuListContext *>(rawContext);
  return context.showingCategories
             ? categories[index].name
             : categories[context.category].items[index].name;
}

uint8_t normalizeMenuState(int requestedIndex) {
  if (menuLevel > MENU_LEVEL_APPLICATIONS) {
    menuLevel = MENU_LEVEL_CATEGORIES;
  }
  if (menuCategory >= categoryCount) {
    menuCategory = 0;
  }
  int itemCount = menuLevel == MENU_LEVEL_CATEGORIES
                      ? categoryCount
                      : categories[menuCategory].itemCount;
  return itemCount > 0 && requestedIndex >= 0
             ? static_cast<uint8_t>(requestedIndex % itemCount)
             : 0;
}

void drawMenu(bool showingCategories, uint8_t category,
              uint8_t selectedIndex) {
  MenuListContext context{showingCategories, category};
  WatchyUi::ListModel model{
      showingCategories ? "CATEGORIES" : categories[category].name,
      menuLabel,
      nullptr,
      &context,
      "UP/DOWN MOVE  SELECT OPEN",
      static_cast<uint8_t>(showingCategories
                               ? categoryCount
                               : categories[category].itemCount),
      selectedIndex,
      WatchyUi::Theme::listVisibleRows,
      static_cast<int16_t>(showingCategories ? category : -1),
      showingCategories,
      false};
  WatchyUi::ListView::draw(model);
}

void updateMenuSelection(bool showingCategories, uint8_t category,
                         uint8_t previousIndex, uint8_t selectedIndex) {
  MenuListContext context{showingCategories, category};
  WatchyUi::ListModel model{
      showingCategories ? "CATEGORIES" : categories[category].name,
      menuLabel,
      nullptr,
      &context,
      "UP/DOWN MOVE  SELECT OPEN",
      static_cast<uint8_t>(showingCategories
                               ? categoryCount
                               : categories[category].itemCount),
      selectedIndex,
      WatchyUi::Theme::listVisibleRows,
      static_cast<int16_t>(showingCategories ? category : -1),
      showingCategories,
      false};
  WatchyUi::ListView::presentSelectionChange(
      model, previousIndex, MAIN_MENU_STATE);
}

} // namespace

int getMenuCategoryCount() {
  return categoryCount;
}

int getMenuItemCount(uint8_t categoryIndex) {
  return categoryIndex < categoryCount ? categories[categoryIndex].itemCount
                                       : 0;
}

const char *getMenuCategoryName(uint8_t categoryIndex) {
  return categoryIndex < categoryCount ? categories[categoryIndex].name : "";
}

const char *getMenuItemName(uint8_t categoryIndex, uint8_t itemIndex) {
  return categoryIndex < categoryCount &&
                 itemIndex < categories[categoryIndex].itemCount
             ? categories[categoryIndex].items[itemIndex].name
             : "";
}

MenuAction getMenuAction(uint8_t categoryIndex, uint8_t itemIndex) {
  if (categoryIndex >= categoryCount ||
      itemIndex >= categories[categoryIndex].itemCount) {
    return MENU_ACTION_ABOUT;
  }
  return categories[categoryIndex].items[itemIndex].action;
}

bool menuActionsComplete() {
  constexpr uint16_t actionCount = MENU_ACTION_BALANCE_CHALLENGE + 1;
  bool seen[actionCount] = {};
  for (uint8_t category = 0; category < categoryCount; category++) {
    for (uint8_t item = 0; item < categories[category].itemCount; item++) {
      uint16_t action = categories[category].items[item].action;
      if (action >= actionCount || seen[action]) {
        return false;
      }
      seen[action] = true;
    }
  }
  for (uint16_t action = 0; action < actionCount; action++) {
    if (!seen[action]) {
      return false;
    }
  }
  return true;
}

namespace {

void showMenuImpl(byte selectedIndex, Watchy *watchy) {
  selectedIndex = normalizeMenuState(selectedIndex);
  menuIndex = selectedIndex;
  drawMenu(menuLevel == MENU_LEVEL_CATEGORIES, menuCategory, selectedIndex);

  WatchyUi::Screen::present(MAIN_MENU_STATE);
  WatchyUi::Input::begin();
  alreadyInMenu = false;
}

void showFastMenuImpl(byte selectedIndex, byte previousIndex) {
  selectedIndex = normalizeMenuState(selectedIndex);
  menuIndex = selectedIndex;
  updateMenuSelection(menuLevel == MENU_LEVEL_CATEGORIES, menuCategory,
                      previousIndex, selectedIndex);
}

} // namespace

void Watchy::showMenu(byte selectedIndex, bool /*partialRefresh*/) {
  showMenuImpl(selectedIndex, this);
}

void Watchy::showFastMenu(byte selectedIndex, byte previousIndex) {
  showFastMenuImpl(selectedIndex, previousIndex);
}

void WatchySdk::showMenu(byte selectedIndex, bool /*partialRefresh*/) {
  showMenuImpl(selectedIndex, nullptr);
}

void WatchySdk::showFastMenu(byte selectedIndex, byte previousIndex) {
  showFastMenuImpl(selectedIndex, previousIndex);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderMenuPreview(int8_t categoryIndex) {
  bool showingCategories = categoryIndex < 0;
  uint8_t category = showingCategories ? 0 : categoryIndex;
  if (category >= categoryCount) {
    category = 0;
  }
  drawMenu(showingCategories, category, 0);
  WatchyUi::Screen::present(MAIN_MENU_STATE);
}

} // namespace WatchyDemo
#endif
