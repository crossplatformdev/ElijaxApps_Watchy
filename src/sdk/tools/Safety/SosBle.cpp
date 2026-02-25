#include "SafetyToolApps.h"

#include "WatchyUi.h"
#include "Watchy.h"

#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include <string>

#include "AppDisplay.h"
#include "EmergencyProfile.h"
#include "SafetySupport.h"
#include "WatchyPowerDiagnostics.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "GalleryFixtures.h"
#endif

namespace WatchySafetyTools {
namespace {

void drawSosBeacon(bool active, const EmergencyProfile::Data &profile) {
  beginAppDisplay("SOS BLE BEACON");
  AppVisual::drawStatusIcon({79, 36, 42, 42},
                            active ? AppVisual::StatusIcon::RADIO
                                   : AppVisual::StatusIcon::EMPTY,
                            true);
  WatchyUi::Canvas::centeredText({0, 90, 200, 19},
                                 active ? "BROADCASTING" : "PAUSED", 2,
                                 WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(130, "BLOOD", clipped(profile.bloodType).c_str(), true);
  AppVisual::drawDataRow(153, "ICE", clipped(profile.icePhone).c_str());
  AppVisual::drawDataRow(176, "RANGE", "Nearby BLE only");
  WatchyUi::Widget::footer("SELECT PAUSE  BACK STOP");
  finishAppDisplay();
}

} // namespace

void runSosBle() {
  WatchyUi::Input::begin();
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  WatchyDiagnostics::beginBleSession();
  Watchy::setRadioCpuFrequency();
  BLEDevice::init("WATCHY SOS");
  BLE_CONFIGURED = true;
  BLEAdvertisementData data;
  data.setName("WATCHY SOS");
  std::string payload = "SOS|";
  payload += profile.bloodType.substring(0, 5).c_str();
  payload += '|';
  payload += profile.icePhone.substring(0, 14).c_str();
  data.setManufacturerData(payload);
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  advertising->setScanResponse(false);
  advertising->setAdvertisementData(data);
  advertising->start();
  bool active = true;
  drawSosBeacon(active, profile);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) break;
    if (event == WatchyUi::Event::MENU) {
      active = !active;
      if (active) advertising->start();
      else advertising->stop();
      drawSosBeacon(active, profile);
    }
  }
  advertising->stop();
  BLEDevice::deinit(false);
  btStop();
  WatchyDiagnostics::endBleSession();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderSosBlePreview(uint8_t view) {
  EmergencyProfile::Data profile = WatchyDemo::GalleryFixtures::emergencyProfile();
  drawSosBeacon(view == 0, profile);
}
#endif

} // namespace WatchySafetyTools
