#include <Watchy.h>
#include "MenuModel.h"
#include "sdk/WatchyUi.h"

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

const MenuItem systemItems[] = {
    {"About Watchy", MENU_ACTION_ABOUT},
    {"Set Time", MENU_ACTION_SET_TIME},
    {"Setup WiFi", MENU_ACTION_SETUP_WIFI},
    {"Watch Faces", MENU_ACTION_WATCH_FACES},
    {"Theme Colours", MENU_ACTION_THEME_COLOURS}};

const MenuItem utilityItems[] = {
    {"Vibrate Motor", MENU_ACTION_VIBRATE},
    {"Accelerometer", MENU_ACTION_ACCELEROMETER},
  {"Sync NTP", MENU_ACTION_SYNC_NTP},
  {"Coin Flip", MENU_ACTION_COIN_FLIP},
  {"D6 Dice", MENU_ACTION_D6_DICE},
  {"D20 Dice", MENU_ACTION_D20_DICE},
  {"Random Number", MENU_ACTION_RANDOM_NUMBER},
  {"Decision Maker", MENU_ACTION_DECISION_MAKER},
  {"Password Gen", MENU_ACTION_PASSWORD_GENERATOR},
  {"UUID Generator", MENU_ACTION_UUID_GENERATOR},
  {"I2C Scanner", MENU_ACTION_I2C_SCANNER},
  {"Chip Info", MENU_ACTION_CHIP_INFO},
  {"Heap Monitor", MENU_ACTION_HEAP_MONITOR},
  {"Wake Reason", MENU_ACTION_WAKE_REASON},
  {"Reset Reason", MENU_ACTION_RESET_REASON},
  {"Button Tester", MENU_ACTION_BUTTON_TESTER},
  {"Vibration Lab", MENU_ACTION_VIBRATION_LAB},
  {"Screen Ruler", MENU_ACTION_SCREEN_RULER},
  {"Temperature Conv", MENU_ACTION_TEMPERATURE_CONVERTER},
  {"Length Converter", MENU_ACTION_LENGTH_CONVERTER},
  {"Weight Converter", MENU_ACTION_WEIGHT_CONVERTER},
  {"Base Converter", MENU_ACTION_BASE_CONVERTER},
  {"Pace Converter", MENU_ACTION_PACE_CONVERTER}};

const MenuItem networkingItems[] = {
    {"Browser", MENU_ACTION_BROWSER},
    {"RSS Feed", MENU_ACTION_RSS_FEED},
    {"Ping", MENU_ACTION_PING},
    {"Traceroute", MENU_ACTION_TRACEROUTE},
    {"Port Scanner", MENU_ACTION_PORT_SCANNER},
    {"DNS Query", MENU_ACTION_DNS_QUERY},
    {"Reverse DNS", MENU_ACTION_REVERSE_DNS},
    {"DuckDuckGo", MENU_ACTION_DUCKDUCKGO},
    {"WiFi Survey", MENU_ACTION_WIFI_SURVEY}};

const MenuItem astronomyItems[] = {
    {"Sun Rise", MENU_ACTION_SUN_RISE},
    {"Moon Rise", MENU_ACTION_MOON_RISE},
    {"Moon Phase", MENU_ACTION_MOON_PHASE},
    {"Tides", MENU_ACTION_TIDES}};

const MenuItem healthItems[] = {
  {"Heart Rate", MENU_ACTION_HEART_RATE},
  {"UN Dog Plate", MENU_ACTION_EMERGENCY_PLATE},
  {"Medical ID", MENU_ACTION_MEDICAL_ID},
  {"ICE Contact", MENU_ACTION_ICE_CONTACT},
  {"Blood Type", MENU_ACTION_BLOOD_TYPE},
  {"Allergies", MENU_ACTION_ALLERGIES},
  {"Medications", MENU_ACTION_MEDICATIONS},
  {"Conditions", MENU_ACTION_CONDITIONS},
  {"Edit Medical ID", MENU_ACTION_EDIT_MEDICAL_ID},
  {"Fall Detector", MENU_ACTION_FALL_DETECTOR},
  {"Body Position", MENU_ACTION_BODY_POSITION},
  {"Saved Location", MENU_ACTION_CONFIGURED_LOCATION},
  {"SOS Screen", MENU_ACTION_SOS_SCREEN},
  {"SOS BLE Beacon", MENU_ACTION_SOS_BLE},
  {"Check-In Timer", MENU_ACTION_CHECK_IN_TIMER},
  {"Medication Alert", MENU_ACTION_MEDICATION_REMINDER},
  {"Hydration Alert", MENU_ACTION_HYDRATION_REMINDER},
  {"Breathing Coach", MENU_ACTION_BREATHING_COACH},
  {"CPR Metronome", MENU_ACTION_CPR_METRONOME},
  {"Recovery Pos.", MENU_ACTION_RECOVERY_POSITION},
  {"Stroke FAST", MENU_ACTION_STROKE_FAST},
  {"Choking Response", MENU_ACTION_CHOKING_RESPONSE},
  {"Seizure Aid", MENU_ACTION_SEIZURE_AID},
  {"Severe Bleeding", MENU_ACTION_SEVERE_BLEEDING},
  {"Burn First Aid", MENU_ACTION_BURN_FIRST_AID},
  {"Heat Emergency", MENU_ACTION_HEAT_EMERGENCY},
  {"Hypothermia", MENU_ACTION_HYPOTHERMIA},
  {"Poisoning", MENU_ACTION_POISONING},
  {"Anaphylaxis", MENU_ACTION_ANAPHYLAXIS},
  {"Opioid Overdose", MENU_ACTION_OPIOID_OVERDOSE},
  {"Asthma Attack", MENU_ACTION_ASTHMA_ATTACK},
  {"Emergency Nos.", MENU_ACTION_EMERGENCY_NUMBERS},
  {"Pain Log", MENU_ACTION_PAIN_LOG},
  {"Symptom Note", MENU_ACTION_SYMPTOM_NOTE},
  {"5-4-3-2-1 Calm", MENU_ACTION_GROUNDING}};

const MenuItem gameItems[] = {
    {"Morse Letter", MENU_ACTION_MORSE_LETTER},
    {"Morse Code", MENU_ACTION_MORSE_CODE},
    {"Pong", MENU_ACTION_PONG},
    {"Snake", MENU_ACTION_SNAKE},
    {"Othello", MENU_ACTION_OTHELLO},
    {"Rock Paper Sciss", MENU_ACTION_ROCK_PAPER_SCISSORS},
    {"Reaction Test", MENU_ACTION_REACTION_TEST},
    {"Higher Lower", MENU_ACTION_HIGHER_LOWER},
    {"Number Guess", MENU_ACTION_NUMBER_GUESS},
    {"Nim", MENU_ACTION_NIM},
    {"Tic Tac Toe", MENU_ACTION_TIC_TAC_TOE},
    {"Lights Out", MENU_ACTION_LIGHTS_OUT},
    {"Blackjack", MENU_ACTION_BLACKJACK},
    {"Quick Math", MENU_ACTION_QUICK_MATH},
    {"Balance", MENU_ACTION_BALANCE_CHALLENGE}};

const MenuItem clockItems[] = {
  {"Binary Clock", MENU_ACTION_BINARY_CLOCK},
  {"Unix Time", MENU_ACTION_UNIX_TIME},
  {"UTC Clock", MENU_ACTION_UTC_CLOCK},
  {"ISO Week", MENU_ACTION_WEEK_NUMBER},
  {"Day of Year", MENU_ACTION_DAY_OF_YEAR},
  {"Calendar", MENU_ACTION_MONTH_CALENDAR},
  {"World Clocks", MENU_ACTION_WORLD_CLOCKS},
  {"Local + UTC", MENU_ACTION_DUAL_TIME},
  {"Internet Beats", MENU_ACTION_INTERNET_BEATS},
  {"Decimal Time", MENU_ACTION_DECIMAL_TIME},
  {"Julian Day", MENU_ACTION_JULIAN_DAY},
  {"Time Progress", MENU_ACTION_TIME_PROGRESS}};

const MenuItem timeToolItems[] = {
  {"Stopwatch", MENU_ACTION_STOPWATCH},
  {"Countdown", MENU_ACTION_COUNTDOWN},
  {"Daily Alarm", MENU_ACTION_DAILY_ALARM},
  {"Pomodoro", MENU_ACTION_POMODORO},
  {"Intervals", MENU_ACTION_INTERVAL_TIMER},
  {"Metronome", MENU_ACTION_METRONOME}};

const MenuItem sensorItems[] = {
  {"Battery Gauge", MENU_ACTION_BATTERY_GAUGE},
  {"Power Budget", MENU_ACTION_POWER_BUDGET},
  {"Charge Status", MENU_ACTION_CHARGE_STATUS},
  {"BMA Temperature", MENU_ACTION_BMA_TEMPERATURE},
  {"Raw Accel", MENU_ACTION_RAW_ACCEL},
  {"G Force", MENU_ACTION_G_FORCE},
  {"Spirit Level", MENU_ACTION_SPIRIT_LEVEL},
  {"Orientation", MENU_ACTION_ORIENTATION},
  {"Motion Score", MENU_ACTION_MOTION_SCORE},
  {"Step Counter", MENU_ACTION_STEP_COUNTER},
  {"Step Goal", MENU_ACTION_STEP_GOAL},
  {"Walk Distance", MENU_ACTION_WALK_DISTANCE},
  {"Step Calories", MENU_ACTION_STEP_CALORIES},
  {"Activity State", MENU_ACTION_ACTIVITY_STATE},
  {"Sensor Status", MENU_ACTION_SENSOR_STATUS},
  {"Uptime", MENU_ACTION_UPTIME},
  {"Shake Counter", MENU_ACTION_SHAKE_COUNTER}};

const MenuItem bluetoothItems[] = {
  {"BLE Scanner", MENU_ACTION_BLE_SCANNER},
  {"Device Count", MENU_ACTION_BLE_DEVICE_COUNT},
  {"Strongest Signal", MENU_ACTION_BLE_STRONGEST},
  {"Named Devices", MENU_ACTION_BLE_NAMED},
  {"Service UUIDs", MENU_ACTION_BLE_SERVICES},
  {"Manufacturer IDs", MENU_ACTION_BLE_MANUFACTURERS},
  {"RSSI Bands", MENU_ACTION_BLE_RSSI_BANDS},
  {"BLE Addresses", MENU_ACTION_BLE_ADDRESSES},
  {"BLE Radar", MENU_ACTION_BLE_RADAR},
  {"TX Power Survey", MENU_ACTION_BLE_TX_POWER},
  {"iBeacon Watch", MENU_ACTION_BLE_IBEACONS},
  {"Watchy Beacon", MENU_ACTION_BLE_BEACON},
  {"Battery Beacon", MENU_ACTION_BLE_BATTERY_BEACON},
  {"Time Beacon", MENU_ACTION_BLE_TIME_BEACON},
  {"Step Beacon", MENU_ACTION_BLE_STEP_BEACON},
  {"Name Badge", MENU_ACTION_BLE_NAME_BADGE}};

const MenuCategory categories[] = {
    {"System", systemItems, sizeof(systemItems) / sizeof(systemItems[0])},
    {"Utilities", utilityItems,
     sizeof(utilityItems) / sizeof(utilityItems[0])},
    {"Networking", networkingItems,
     sizeof(networkingItems) / sizeof(networkingItems[0])},
    {"Astronomy", astronomyItems,
     sizeof(astronomyItems) / sizeof(astronomyItems[0])},
    {"Healthcare", healthItems, sizeof(healthItems) / sizeof(healthItems[0])},
    {"Games", gameItems, sizeof(gameItems) / sizeof(gameItems[0])},
    {"Clocks", clockItems, sizeof(clockItems) / sizeof(clockItems[0])},
    {"Time Tools", timeToolItems,
      sizeof(timeToolItems) / sizeof(timeToolItems[0])},
    {"Sensors", sensorItems, sizeof(sensorItems) / sizeof(sensorItems[0])},
    {"Bluetooth", bluetoothItems,
     sizeof(bluetoothItems) / sizeof(bluetoothItems[0])}};

constexpr int categoryCount = sizeof(categories) / sizeof(categories[0]);
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
      nullptr,
      static_cast<uint8_t>(showingCategories
                               ? categoryCount
                               : categories[menuCategory].itemCount),
      selectedIndex,
      WatchyUi::Theme::listVisibleRows,
      -1,
      showingCategories};
  WatchyUi::ListView::draw(model);
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

void Watchy::showMenu(byte selectedIndex, bool /*partialRefresh*/) {
  selectedIndex = normalizeMenuState(selectedIndex);
  menuIndex = selectedIndex;
  drawMenu(menuLevel == MENU_LEVEL_CATEGORIES, menuCategory, selectedIndex);

  WatchyUi::Screen::present(MAIN_MENU_STATE);
  alreadyInMenu = false;
}

void Watchy::showFastMenu(byte selectedIndex) {
  selectedIndex = normalizeMenuState(selectedIndex);
  menuIndex = selectedIndex;
  drawMenu(menuLevel == MENU_LEVEL_CATEGORIES, menuCategory, selectedIndex);

  WatchyUi::Screen::present(MAIN_MENU_STATE);
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