#include "DeterministicGallery.h"
#include "WatchyUi.h"
#include "Watchy.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

#include "HeartRate.h"
#include "GalleryRenderers.h"
#include "ScreenCapture.h"
#include "MenuModel.h"

#include "WatchFaceRegistry.h"

namespace WatchyDemo {

namespace {

enum class Renderer : uint8_t {
  CLOCK, TIME_TOOL, SENSOR, UTILITY, MORSE_LETTER, MORSE_CODE, PONG,
  SNAKE, OTHELLO, MINI_GAME, HEART_RATE, HEALTHCARE, SAFETY,
  HEALTH_REMINDER, HEALTH_SUPPORT, ABOUT, SET_TIME, WIFI_SETUP,
  WATCHFACE_SELECTOR, THEME, BUZZ, ACCELEROMETER, SYNC_NTP, SUN_RISE,
  MOON_RISE, MOON_PHASE, TIDES, NETWORK, BLUETOOTH
};

struct GalleryApp {
  Renderer renderer;
  uint8_t tool;
  const char *prefix;
  const char *states[5];
};

#define GALLERY_APP(renderer, tool, prefix, state1, state2, state3, state4, state5) \
  {Renderer::renderer, tool, prefix, {state1, state2, state3, state4, state5}},
const GalleryApp galleryApps[] = {
#include "GalleryAppCatalog.inc"
};
#undef GALLERY_APP

const char *const calibrationSceneIds[] = {
  "sdk/grayscale/ramp/light",
  "sdk/grayscale/semantic-tones/light",
  "sdk/grayscale/semantic-tones/dark",
  "sdk/grayscale/widgets/light",
  "sdk/grayscale/widgets/dark",
  "sdk/grayscale/dialog/light",
  "sdk/grayscale/list/light",
  "sdk/grayscale/disabled-controls/light",
  "sdk/grayscale/graph/light"};

const char *const watchfaceSceneIds[] = {
    "watchfaces/7-seg/light", "watchfaces/7-seg/dark",
    "watchfaces/basic/light", "watchfaces/basic/dark",
    "watchfaces/dos/light", "watchfaces/dos/dark",
    "watchfaces/macpaint/light", "watchfaces/macpaint/dark",
    "watchfaces/mario/light", "watchfaces/mario/dark",
    "watchfaces/pokemon/light", "watchfaces/pokemon/dark",
    "watchfaces/starry-horizon/light", "watchfaces/starry-horizon/dark",
    "watchfaces/tetris/light", "watchfaces/tetris/dark"};

const char *const menuSceneIds[] = {
    "os/menu/categories/light",
  "os/menu/clocks-sky/light",
  "os/menu/timers-focus/light",
  "os/menu/health-wellness/light",
  "os/menu/safety-first-aid/light",
  "os/menu/sensors-activity/light",
  "os/menu/everyday-tools/light",
  "os/menu/games-puzzles/light",
  "os/menu/network-web/light",
  "os/menu/bluetooth/light",
  "os/menu/watch-system/light"};

template <typename Value, size_t Count>
constexpr size_t arraySize(const Value (&)[Count]) {
  return Count;
}

constexpr uint16_t applicationCount = arraySize(galleryApps);
#define GALLERY_APP(renderer, tool, prefix, state1, state2, state3, state4, state5) \
  + (state1 != nullptr) + (state2 != nullptr) + (state3 != nullptr) + \
    (state4 != nullptr) + (state5 != nullptr)
constexpr uint16_t applicationSceneCount = 0
#include "GalleryAppCatalog.inc"
;
#undef GALLERY_APP
constexpr uint16_t calibrationSceneCount = arraySize(calibrationSceneIds);
constexpr uint16_t watchfaceSceneCount = arraySize(watchfaceSceneIds);
constexpr uint16_t navigationSceneCount = arraySize(menuSceneIds);
constexpr uint16_t totalSceneCount =
  applicationSceneCount + calibrationSceneCount + watchfaceSceneCount +
  navigationSceneCount;

static_assert(applicationCount == 142,
              "Gallery catalog must cover all registered applications");
static_assert(applicationCount == MENU_ACTION_BALANCE_CHALLENGE + 1,
              "Gallery scene count must match MenuAction");
static_assert(navigationSceneCount == MENU_CATEGORY_COUNT + 1,
              "Gallery must cover root and category menus");
static_assert(calibrationSceneCount == 9,
              "Gallery must cover Gray8 and SDK widget calibration");
static_assert(watchfaceSceneCount == WATCHFACE_COUNT * 2,
              "Gallery must cover every WatchFace in light and dark themes");
static_assert(applicationSceneCount >= applicationCount,
              "Every application needs at least one gallery view");

void renderApp(const GalleryApp &app, uint8_t view,
               const tmElements_t &fixedTime) {
  switch (app.renderer) {
  case Renderer::CLOCK: renderClockPreview(app.tool, view, fixedTime); break;
  case Renderer::TIME_TOOL: renderTimeToolPreview(app.tool, view); break;
  case Renderer::SENSOR: renderSensorPreview(app.tool, view); break;
  case Renderer::UTILITY: renderUtilityPreview(app.tool, view); break;
  case Renderer::MORSE_LETTER: renderMorseLetterPreview(view); break;
  case Renderer::MORSE_CODE: renderMorseCodePreview(view); break;
  case Renderer::PONG: renderPongPreview(view); break;
  case Renderer::SNAKE: renderSnakePreview(view); break;
  case Renderer::OTHELLO: renderOthelloPreview(view); break;
  case Renderer::MINI_GAME: renderMiniGamePreview(app.tool, view); break;
  case Renderer::HEART_RATE: renderHeartRatePreview(view); break;
  case Renderer::HEALTHCARE: renderHealthcarePreview(app.tool, view); break;
  case Renderer::SAFETY: renderSafetyPreview(app.tool, view); break;
  case Renderer::HEALTH_REMINDER: renderHealthReminderPreview(app.tool, view); break;
  case Renderer::HEALTH_SUPPORT: renderHealthSupportPreview(app.tool, view); break;
  case Renderer::ABOUT: renderAboutPreview(view); break;
  case Renderer::SET_TIME: renderSetTimePreview(view); break;
  case Renderer::WIFI_SETUP: renderWifiSetupPreview(view); break;
  case Renderer::WATCHFACE_SELECTOR: renderWatchfaceSelectorPreview(view); break;
  case Renderer::THEME: renderThemePreview(view); break;
  case Renderer::BUZZ: renderBuzzPreview(view); break;
  case Renderer::ACCELEROMETER: renderAccelerometerPreview(view); break;
  case Renderer::SYNC_NTP: renderSyncNtpPreview(view); break;
  case Renderer::SUN_RISE: renderSunRisePreview(view, fixedTime); break;
  case Renderer::MOON_RISE: renderMoonRisePreview(view, fixedTime); break;
  case Renderer::MOON_PHASE: renderMoonPhasePreview(view, fixedTime); break;
  case Renderer::TIDES: renderTidesPreview(view, fixedTime); break;
  case Renderer::NETWORK: renderNetworkPreview(app.tool, view); break;
  case Renderer::BLUETOOTH: renderBluetoothPreview(app.tool, view); break;
  }
}

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

void renderToneCalibration() {
  constexpr WatchyUi::ToneRole roles[] = {
      WatchyUi::ToneRole::Background, WatchyUi::ToneRole::Surface,
      WatchyUi::ToneRole::SurfaceRaised, WatchyUi::ToneRole::Foreground,
      WatchyUi::ToneRole::SecondaryText, WatchyUi::ToneRole::Muted,
      WatchyUi::ToneRole::Disabled, WatchyUi::ToneRole::Separator,
      WatchyUi::ToneRole::Accent, WatchyUi::ToneRole::Selection};
  WatchyUi::Screen::beginCanvas();
  for (uint8_t index = 0; index < arraySize(roles); index++) {
    WatchyUi::GrayPaint::fillRect(
        {4, static_cast<int16_t>(index * 20), 192, 20},
        WatchyUi::Theme::tone(roles[index]));
  }
  WatchyUi::Screen::present();
}

void renderWidgetCalibration() {
  WatchyUi::Screen::begin("WIDGET STATES");
  WatchyUi::Widget::separator();
  WatchyUi::Widget::checkbox("UNCHECKED", false, 42);
  WatchyUi::Widget::checkbox("CHECKED", true, 66, true);
  WatchyUi::Widget::radio("RADIO", true, 92);
  WatchyUi::Widget::toggle(false, 24, 112);
  WatchyUi::Widget::toggle(true, 74, 112, true);
  WatchyUi::Widget::progress(0.62f, 145);
  WatchyUi::Widget::footer("SEMANTIC TONES");
  WatchyUi::Screen::present();
}

void renderListCalibration() {
  static const char *const labels[] = {
      "Selected row", "Normal row", "Secondary", "Disabled", "More"};
  WatchyUi::ListView::draw("GRAY LIST", labels, arraySize(labels), 0,
                           "UP/DOWN SELECT", 2, 5);
  WatchyUi::Screen::present();
}

void renderDisabledCalibration() {
  WatchyUi::Screen::begin("CONTROL STATES");
  WatchyUi::Widget::separator();
  WatchyUi::Widget::checkbox("OPTION", false, 48, false, false);
  WatchyUi::Widget::checkbox("ENABLED", true, 76);
  WatchyUi::Widget::radio("CHOICE", false, 106, false, false);
  WatchyUi::Widget::radio("ACTIVE", true, 134);
  WatchyUi::Widget::footer("DISABLED / ENABLED");
  WatchyUi::Screen::present();
}

void renderGraphCalibration() {
  WatchyUi::Screen::begin("GRAY GRAPH");
  WatchyUi::Widget::separator();
  constexpr WatchyUi::Bounds chart{8, 34, 184, 132};
  WatchyUi::GrayPaint::fillRect(
      chart, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  for (int16_t y = 56; y < 166; y += 22) {
    WatchyUi::GrayPaint::line(
        chart.x, y, chart.x + chart.width - 1, y,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Separator));
  }
  constexpr int16_t values[] = {142, 126, 132, 96, 108, 72, 88, 52, 64};
  for (uint8_t index = 1; index < arraySize(values); index++) {
    WatchyUi::GrayPaint::line(
        12 + (index - 1) * 21, values[index - 1],
        12 + index * 21, values[index],
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Accent));
  }
  WatchyUi::Widget::footer("STATIC SPATIAL DITHER");
  WatchyUi::Screen::present();
}

void renderGrayscaleCalibration(uint8_t scene) {
  WatchyUi::Theme::useGalleryTheme(scene == 2 || scene == 4);
  switch (scene) {
  case 0:
    WatchyUi::Screen::beginCanvas();
    WatchyUi::GrayPaint::gradient({4, 4, 192, 92}, 0, 255);
    for (uint8_t index = 0; index < 16; index++) {
      WatchyUi::GrayPaint::fillRect(
          {static_cast<int16_t>(4 + index * 12), 104, 12, 92},
          static_cast<WatchyUi::Gray8>(index * 255 / 15));
    }
    WatchyUi::Screen::present();
    break;
  case 1:
  case 2: renderToneCalibration(); break;
  case 3:
  case 4: renderWidgetCalibration(); break;
  case 5:
    WatchyUi::Feedback::showMessage(
        "GRAY DIALOG", "Deterministic surface, border and message state.",
        WatchyUi::MessageKind::INFO, "SELECT OK     BACK");
    break;
  case 6: renderListCalibration(); break;
  case 7: renderDisabledCalibration(); break;
  case 8: renderGraphCalibration(); break;
  }
}

} // namespace

void runDeterministicGallery() {
  ScreenCapture::begin();
  WatchyUi::Theme::useGalleryTheme(false);

  uint16_t registeredApplications = 0;
  for (uint8_t category = 0; category < getMenuCategoryCount(); category++) {
    registeredApplications += getMenuItemCount(category);
  }
  if (registeredApplications != applicationCount) {
    ScreenCapture::error("menu-count", applicationCount,
                         registeredApplications);
    ScreenCapture::finish(totalSceneCount);
    return;
  }
  if (!menuActionsComplete()) {
    ScreenCapture::error("menu-actions", applicationCount,
                         registeredApplications);
    ScreenCapture::finish(totalSceneCount);
    return;
  }

  tmElements_t fixedTime = galleryTime();
  for (uint8_t scene = 0; scene < calibrationSceneCount; scene++) {
    ScreenCapture::arm(calibrationSceneIds[scene]);
    renderGrayscaleCalibration(scene);
  }
  bool previousWifiConfigured = WIFI_CONFIGURED;
  bool previousBleConfigured = BLE_CONFIGURED;
  bool previousUsbPluggedIn = USB_PLUGGED_IN;
  uint8_t previousHeartRate = heartRateBpm;
  bool previousHeartRateValid = heartRateValid;
  WIFI_CONFIGURED = true;
  BLE_CONFIGURED = false;
  USB_PLUGGED_IN = false;
  heartRateBpm = 72;
  heartRateValid = true;
  for (uint8_t scene = 0; scene < watchfaceSceneCount; scene++) {
    bool dark = (scene & 1U) != 0;
    uint8_t watchfaceId = scene / 2;
    WatchyUi::Theme::useGalleryTheme(dark);
    ScreenCapture::arm(watchfaceSceneIds[scene]);
    WatchyUi::Screen::beginCanvas();
    WatchySdk::drawGalleryWatchface(watchfaceId, fixedTime);
    WatchyUi::Screen::present(WATCHFACE_STATE);
  }
  WIFI_CONFIGURED = previousWifiConfigured;
  BLE_CONFIGURED = previousBleConfigured;
  USB_PLUGGED_IN = previousUsbPluggedIn;
  heartRateBpm = previousHeartRate;
  heartRateValid = previousHeartRateValid;
  WatchyUi::Theme::useGalleryTheme(false);
  for (uint8_t scene = 0; scene < navigationSceneCount; scene++) {
    ScreenCapture::arm(menuSceneIds[scene]);
    renderMenuPreview(scene == 0 ? -1 : scene - 1);
  }
  char sceneId[96];
  for (const GalleryApp &app : galleryApps) {
    for (uint8_t view = 0; view < 5 && app.states[view] != nullptr; view++) {
      snprintf(sceneId, sizeof(sceneId), "%s/%s/light", app.prefix,
               app.states[view]);
      ScreenCapture::arm(sceneId);
      renderApp(app, view, fixedTime);
    }
  }
  ScreenCapture::finish(totalSceneCount);

}

} // namespace WatchyDemo

#endif
