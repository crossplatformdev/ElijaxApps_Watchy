#include "DeterministicGallery.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

#include <Watchy.h>
#include "GalleryRenderers.h"
#include "ScreenCapture.h"
#include "os/MenuModel.h"
#include "sdk/WatchyUi.h"

namespace WatchyDemo {

namespace {

const char *const clockSceneIds[] = {
    "clocks/binary-clock/default/light",
    "clocks/unix-time/default/light",
    "clocks/utc-clock/default/light",
    "clocks/iso-week/default/light",
    "clocks/day-of-year/default/light",
    "clocks/month-calendar/default/light",
    "clocks/world-clocks/default/light",
    "clocks/local-utc/default/light",
    "clocks/internet-beats/default/light",
    "clocks/decimal-time/default/light",
    "clocks/julian-day/default/light",
    "clocks/time-progress/default/light"};

const char *const timeToolSceneIds[] = {
    "time-tools/stopwatch/running/light",
    "time-tools/countdown/running/light",
    "time-tools/daily-alarm/enabled/light",
    "time-tools/pomodoro/focus/light",
    "time-tools/intervals/work/light",
    "time-tools/metronome/running/light"};

  const char *const sensorSceneIds[] = {
    "sensors/battery-gauge/default/light",
    "sensors/power-budget/default/light",
    "sensors/charge-status/charging/light",
    "sensors/bma-temperature/default/light",
    "sensors/raw-accel/default/light",
    "sensors/g-force/default/light",
    "sensors/spirit-level/default/light",
    "sensors/orientation/face-up/light",
    "sensors/motion-score/moving/light",
    "sensors/step-counter/default/light",
    "sensors/step-goal/default/light",
    "sensors/walk-distance/default/light",
    "sensors/step-calories/default/light",
    "sensors/activity/walking/light",
    "sensors/sensor-status/healthy/light",
    "sensors/uptime/default/light",
    "sensors/shake-counter/default/light"};

  const char *const utilitySceneIds[] = {
    "utilities/coin-flip/heads/light",
    "utilities/d6-dice/four/light",
    "utilities/d20-dice/seventeen/light",
    "utilities/random-number/default/light",
    "utilities/decision-maker/yes/light",
    "utilities/password-generator/default/light",
    "utilities/uuid-generator/default/light",
    "utilities/i2c-scanner/default/light",
    "utilities/chip-info/default/light",
    "utilities/heap-monitor/default/light",
    "utilities/wake-reason/timer/light",
    "utilities/reset-reason/power-on/light",
    "utilities/button-tester/default/light",
    "utilities/vibration-lab/heartbeat/light",
    "utilities/screen-ruler/default/light",
    "utilities/temperature-converter/default/light",
    "utilities/length-converter/default/light",
    "utilities/weight-converter/default/light",
    "utilities/base-converter/default/light",
    "utilities/pace-converter/default/light"};

  const char *const gameSceneIds[] = {
    "games/morse-letter/answered/light",
    "games/morse-code/answered/light",
    "games/pong/in-play/light",
    "games/snake/in-play/light",
    "games/othello/in-play/light",
    "games/rock-paper-scissors/win/light",
    "games/reaction-test/result/light",
    "games/higher-lower/correct/light",
    "games/number-guess/too-low/light",
    "games/nim/in-play/light",
    "games/tic-tac-toe/in-play/light",
    "games/lights-out/in-play/light",
    "games/blackjack/in-play/light",
    "games/quick-math/correct/light",
    "games/balance/in-play/light"};

  const char *const healthcareSceneIds[] = {
    "healthcare/heart-rate/result/light",
    "healthcare/un-dog-plate/demo/light",
    "healthcare/medical-id/demo/light",
    "healthcare/ice-contact/demo/light",
    "healthcare/blood-type/demo/light",
    "healthcare/allergies/demo/light",
    "healthcare/medications/demo/light",
    "healthcare/conditions/demo/light",
    "healthcare/edit-medical-id/demo/light",
    "healthcare/fall-detector/monitoring/light",
    "healthcare/body-position/face-up/light",
    "healthcare/saved-location/demo/light",
    "healthcare/sos-screen/demo/light",
    "healthcare/sos-ble/broadcasting/light",
    "healthcare/check-in-timer/armed/light",
    "healthcare/medication-alert/enabled/light",
    "healthcare/hydration-alert/enabled/light",
    "healthcare/breathing-coach/inhale/light",
    "healthcare/cpr-metronome/running/light",
    "healthcare/recovery-position/default/light",
    "healthcare/stroke-fast/default/light",
    "healthcare/choking-response/default/light",
    "healthcare/seizure-aid/default/light",
    "healthcare/severe-bleeding/default/light",
    "healthcare/burn-first-aid/default/light",
    "healthcare/heat-emergency/default/light",
    "healthcare/hypothermia/default/light",
    "healthcare/poisoning/default/light",
    "healthcare/anaphylaxis/default/light",
    "healthcare/opioid-overdose/default/light",
    "healthcare/asthma-attack/default/light",
    "healthcare/emergency-numbers/default/light",
    "healthcare/pain-log/default/light",
    "healthcare/symptom-note/demo/light",
    "healthcare/grounding/step-five/light"};

  const char *const systemSceneIds[] = {
    "system/about-watchy/default/light",
    "system/set-time/default/light",
    "system/setup-wifi/connected/light",
    "system/watch-faces/selector/light",
    "system/theme-colours/light-selected/light"};

  const char *const osUtilitySceneIds[] = {
    "utilities/vibrate-motor/default/light",
    "utilities/accelerometer/face-up/light",
    "utilities/sync-ntp/success/light"};

  const char *const astronomySceneIds[] = {
    "astronomy/sun-rise/madrid/light",
    "astronomy/moon-rise/madrid/light",
    "astronomy/moon-phase/default/light",
    "astronomy/tides/brest/light"};

  const char *const networkSceneIds[] = {
    "networking/browser/result/light",
    "networking/rss-feed/result/light",
    "networking/ping/result/light",
    "networking/traceroute/result/light",
    "networking/port-scanner/result/light",
    "networking/dns-query/result/light",
    "networking/reverse-dns/result/light",
    "networking/duckduckgo/result/light",
    "networking/wifi-survey/result/light"};

  const char *const bluetoothSceneIds[] = {
    "bluetooth/ble-scanner/result/light",
    "bluetooth/device-count/result/light",
    "bluetooth/strongest-signal/result/light",
    "bluetooth/named-devices/result/light",
    "bluetooth/service-uuids/result/light",
    "bluetooth/manufacturer-ids/result/light",
    "bluetooth/rssi-bands/result/light",
    "bluetooth/ble-addresses/result/light",
    "bluetooth/ble-radar/result/light",
    "bluetooth/tx-power/result/light",
    "bluetooth/ibeacon-watch/result/light",
    "bluetooth/watchy-beacon/on-air/light",
    "bluetooth/battery-beacon/on-air/light",
    "bluetooth/time-beacon/on-air/light",
    "bluetooth/step-beacon/on-air/light",
    "bluetooth/name-badge/on-air/light"};

  const char *const menuSceneIds[] = {
    "os/menu/categories/light",
    "os/menu/system/light",
    "os/menu/utilities/light",
    "os/menu/networking/light",
    "os/menu/astronomy/light",
    "os/menu/healthcare/light",
    "os/menu/games/light",
    "os/menu/clocks/light",
    "os/menu/time-tools/light",
    "os/menu/sensors/light",
    "os/menu/bluetooth/light"};

  template <typename Value, size_t Count>
  constexpr size_t arraySize(const Value (&)[Count]) {
    return Count;
  }

  constexpr uint16_t applicationSceneCount =
    arraySize(systemSceneIds) + arraySize(utilitySceneIds) +
    arraySize(osUtilitySceneIds) + arraySize(networkSceneIds) +
    arraySize(astronomySceneIds) + arraySize(healthcareSceneIds) +
    arraySize(gameSceneIds) + arraySize(clockSceneIds) +
    arraySize(timeToolSceneIds) + arraySize(sensorSceneIds) +
    arraySize(bluetoothSceneIds);
  constexpr uint16_t navigationSceneCount = arraySize(menuSceneIds);
  constexpr uint16_t totalSceneCount =
    applicationSceneCount + navigationSceneCount;

  static_assert(applicationSceneCount == 142,
          "Gallery must cover all registered applications");
  static_assert(applicationSceneCount == MENU_ACTION_BALANCE_CHALLENGE + 1,
          "Gallery scene count must match MenuAction");
  static_assert(navigationSceneCount == MENU_CATEGORY_COUNT + 1,
          "Gallery must cover root and category menus");
  static_assert(totalSceneCount == 153,
          "Deterministic gallery scene count changed");

tmElements_t galleryTime() {
  tmElements_t fixedTime{};
  fixedTime.Second = 45;
  fixedTime.Minute = 34;
  fixedTime.Hour = 10;
  fixedTime.Wday = 1;
  fixedTime.Day = 23;
  fixedTime.Month = 8;
  fixedTime.Year = CalendarYrToTm(2026);
  return fixedTime;
}

} // namespace

void runDeterministicGallery(Watchy &watch) {
  ScreenCapture::begin();
  WatchyUi::Theme::useGalleryTheme(false);

  uint16_t registeredApplications = 0;
  for (uint8_t category = 0; category < getMenuCategoryCount(); category++) {
    registeredApplications += getMenuItemCount(category);
  }
  if (registeredApplications != applicationSceneCount) {
    ScreenCapture::error("menu-count", applicationSceneCount,
                         registeredApplications);
    ScreenCapture::finish(totalSceneCount);
    return;
  }

  tmElements_t fixedTime = galleryTime();
  for (uint8_t scene = 0; scene < navigationSceneCount; scene++) {
    ScreenCapture::arm(menuSceneIds[scene]);
    renderMenuPreview(scene == 0 ? -1 : scene - 1);
  }
  for (uint8_t tool = 0; tool < 12; tool++) {
    ScreenCapture::arm(clockSceneIds[tool]);
    renderClockPreview(tool, fixedTime);
  }
  for (uint8_t tool = 0; tool < 6; tool++) {
    ScreenCapture::arm(timeToolSceneIds[tool]);
    renderTimeToolPreview(tool);
  }
  for (uint8_t tool = 0; tool < 17; tool++) {
    ScreenCapture::arm(sensorSceneIds[tool]);
    renderSensorPreview(tool);
  }
  for (uint8_t tool = 0; tool < 20; tool++) {
    ScreenCapture::arm(utilitySceneIds[tool]);
    renderUtilityPreview(tool);
  }
  for (uint8_t game = 0; game < 15; game++) {
    ScreenCapture::arm(gameSceneIds[game]);
    switch (game) {
    case 0: renderMorseLetterPreview(); break;
    case 1: renderMorseCodePreview(); break;
    case 2: renderPongPreview(); break;
    case 3: renderSnakePreview(); break;
    case 4: renderOthelloPreview(); break;
    default: renderMiniGamePreview(game - 5); break;
    }
  }
  for (uint8_t tool = 0; tool < 35; tool++) {
    ScreenCapture::arm(healthcareSceneIds[tool]);
    if (tool == 0) renderHeartRatePreview();
    else if (tool <= 8) renderHealthcarePreview(tool - 1);
    else if (tool <= 13) renderSafetyPreview(tool - 9);
    else if (tool <= 16) renderHealthReminderPreview(tool - 14);
    else renderHealthSupportPreview(tool - 17);
  }
  for (uint8_t tool = 0; tool < 5; tool++) {
    ScreenCapture::arm(systemSceneIds[tool]);
    switch (tool) {
    case 0: renderAboutPreview(); break;
    case 1: renderSetTimePreview(); break;
    case 2: renderWifiSetupPreview(); break;
    case 3: renderWatchfaceSelectorPreview(); break;
    case 4: renderThemePreview(); break;
    }
  }
  for (uint8_t tool = 0; tool < 3; tool++) {
    ScreenCapture::arm(osUtilitySceneIds[tool]);
    if (tool == 0) renderBuzzPreview();
    else if (tool == 1) renderAccelerometerPreview();
    else renderSyncNtpPreview();
  }
  for (uint8_t tool = 0; tool < 4; tool++) {
    ScreenCapture::arm(astronomySceneIds[tool]);
    if (tool == 0) renderSunRisePreview(fixedTime);
    else if (tool == 1) renderMoonRisePreview(fixedTime);
    else if (tool == 2) renderMoonPhasePreview(fixedTime);
    else renderTidesPreview(fixedTime);
  }
  for (uint8_t tool = 0; tool < 9; tool++) {
    ScreenCapture::arm(networkSceneIds[tool]);
    renderNetworkPreview(tool);
  }
  for (uint8_t tool = 0; tool < 16; tool++) {
    ScreenCapture::arm(bluetoothSceneIds[tool]);
    renderBluetoothPreview(tool);
  }
  ScreenCapture::finish(totalSceneCount);

  (void)watch;
}

} // namespace WatchyDemo

#endif