#include <Watchy.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include <math.h>
#include <string>
#include "AppDisplay.h"
#include "EmergencyProfile.h"

namespace {

enum SafetyTool : uint8_t {
  FALL_DETECTOR,
  BODY_POSITION,
  CONFIGURED_LOCATION,
  SOS_SCREEN,
  SOS_BLE,
  SAFETY_TOOL_COUNT
};

enum FallState : uint8_t {
  FALL_MONITORING,
  FALL_LOW_GRAVITY,
  FALL_IMPACT,
  FALL_ALERT
};

constexpr float FALL_LSB_PER_G = 256.0f;

float magnitudeG(const Accel &acceleration) {
  float x = acceleration.x / FALL_LSB_PER_G;
  float y = acceleration.y / FALL_LSB_PER_G;
  float z = acceleration.z / FALL_LSB_PER_G;
  return sqrtf(x * x + y * y + z * z);
}

void drawFallState(FallState state) {
  beginAppDisplay("FALL DETECTOR");
  Watchy::display.setFont();
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(5, 62);
  switch (state) {
  case FALL_MONITORING: Watchy::display.println("MONITORING"); break;
  case FALL_LOW_GRAVITY: Watchy::display.println("LOW-G EVENT"); break;
  case FALL_IMPACT: Watchy::display.println("CHECKING STILL"); break;
  case FALL_ALERT: Watchy::display.println("FALL SUSPECTED"); break;
  }
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(5, 105);
  if (state == FALL_ALERT) {
    Watchy::display.println("CHECK PERSON. CALL EMERGENCY");
    Watchy::display.setCursor(5, 120);
    Watchy::display.println("SERVICES IF NEEDED.");
    Watchy::display.setCursor(5, 150);
    Watchy::display.println("SELECT: RESET ALERT");
  } else {
    Watchy::display.println("Active only while this app is open");
    Watchy::display.setCursor(5, 122);
    Watchy::display.println("Low-g + impact + stillness");
  }
  Watchy::display.setCursor(5, 178);
  Watchy::display.println("Not a certified medical device");
  Watchy::display.setCursor(5, 191);
  Watchy::display.println("BACK: STOP");
  finishAppDisplay();
}

void runFallDetector(Watchy &watch) {
  WatchyUi::Input::begin();
  Acfg previousConfig;
  if (!sensor.getAccelConfig(previousConfig)) {
    WatchyUi::Feedback::showMessage(
        "FALL DETECTOR", "BMA configuration unavailable.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    return;
  }
  Acfg fallConfig = previousConfig;
  fallConfig.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  fallConfig.range = BMA4_ACCEL_RANGE_8G;
  fallConfig.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
  fallConfig.perf_mode = BMA4_CONTINUOUS_MODE;
  if (!sensor.setAccelConfig(fallConfig) || !sensor.enableAccel()) {
    sensor.setAccelConfig(previousConfig);
    WatchyUi::Feedback::showMessage(
        "FALL DETECTOR", "BMA setup failed. Previous configuration restored.",
        WatchyUi::MessageKind::ERROR, "BACK EXIT");
    while (WatchyUi::Input::wait() != WatchyUi::Event::BACK) {}
    return;
  }

  FallState state = FALL_MONITORING;
  uint32_t lowGravityStarted = 0;
  uint32_t stateStarted = millis();
  uint32_t stillStarted = 0;
  float previousMagnitude = 1.0f;
  drawFallState(state);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::poll();
    if (event == WatchyUi::Event::BACK) {
      break;
    }
    if (state == FALL_ALERT && event == WatchyUi::Event::SELECT) {
      state = FALL_MONITORING;
      lowGravityStarted = 0;
      stillStarted = 0;
      drawFallState(state);
    }

    Accel acceleration;
    if (!sensor.getAccel(acceleration)) {
      delay(20);
      continue;
    }
    uint32_t now = millis();
    float magnitude = magnitudeG(acceleration);

    if (state == FALL_MONITORING) {
      if (magnitude < 0.45f) {
        if (lowGravityStarted == 0) {
          lowGravityStarted = now;
        } else if (now - lowGravityStarted >= 120) {
          state = FALL_LOW_GRAVITY;
          stateStarted = now;
          drawFallState(state);
        }
      } else {
        lowGravityStarted = 0;
      }
    } else if (state == FALL_LOW_GRAVITY) {
      if (magnitude > 2.5f) {
        state = FALL_IMPACT;
        stateStarted = now;
        stillStarted = 0;
        drawFallState(state);
      } else if (now - stateStarted > 1500) {
        state = FALL_MONITORING;
        lowGravityStarted = 0;
        drawFallState(state);
      }
    } else if (state == FALL_IMPACT) {
      bool still = fabsf(magnitude - 1.0f) < 0.35f &&
                   fabsf(magnitude - previousMagnitude) < 0.10f;
      if (still) {
        if (stillStarted == 0) {
          stillStarted = now;
        } else if (now - stillStarted >= 2500) {
          state = FALL_ALERT;
          drawFallState(state);
          watch.vibMotor(120, 14);
        }
      } else {
        stillStarted = 0;
      }
      if (state == FALL_IMPACT && now - stateStarted > 8000) {
        state = FALL_MONITORING;
        lowGravityStarted = 0;
        drawFallState(state);
      }
    }
    previousMagnitude = magnitude;
    delay(20);
  }
  sensor.setAccelConfig(previousConfig);
}

const char *positionName(uint8_t direction) {
  switch (direction) {
  case DIRECTION_DISP_DOWN: return "FACE DOWN";
  case DIRECTION_DISP_UP: return "FACE UP";
  case DIRECTION_BOTTOM_EDGE: return "BOTTOM EDGE";
  case DIRECTION_TOP_EDGE: return "TOP EDGE";
  case DIRECTION_RIGHT_EDGE: return "RIGHT EDGE";
  case DIRECTION_LEFT_EDGE: return "LEFT EDGE";
  default: return "UNKNOWN";
  }
}

void drawPosition(uint8_t direction, uint32_t faceDownSeconds,
                  bool warning) {
  beginAppDisplay("BODY POSITION");
  Watchy::display.setFont();
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(8, 70);
  Watchy::display.println(positionName(direction));
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 112);
  Watchy::display.print("Face-down time: ");
  Watchy::display.print(faceDownSeconds);
  Watchy::display.println(" s");
  Watchy::display.setCursor(8, 145);
  Watchy::display.println(warning ? "CHECK BREATHING / RESPONSIVENESS"
                                  : "Active posture monitor");
  Watchy::display.setCursor(8, 180);
  Watchy::display.println("BACK: STOP");
  finishAppDisplay();
}

void runPositionMonitor(Watchy &watch) {
  WatchyUi::Input::begin();
  uint8_t previousDirection = UINT8_MAX;
  uint32_t faceDownStarted = 0;
  uint32_t displayedBucket = UINT32_MAX;
  bool warned = false;
  bool displayedWarning = false;
  while (true) {
    if (WatchyUi::Input::poll() == WatchyUi::Event::BACK) {
      return;
    }
    uint8_t direction = sensor.getDirection();
    uint32_t now = millis();
    if (direction == DIRECTION_DISP_DOWN) {
      if (faceDownStarted == 0) {
        faceDownStarted = now;
      }
    } else {
      faceDownStarted = 0;
      warned = false;
    }
    uint32_t seconds = faceDownStarted == 0 ? 0 : (now - faceDownStarted) / 1000;
    if (!warned && seconds >= 30) {
      warned = true;
      watch.vibMotor(100, 8);
    }
    uint32_t bucket = seconds / 5;
    if (direction != previousDirection || bucket != displayedBucket ||
        warned != displayedWarning) {
      previousDirection = direction;
      displayedBucket = bucket;
      displayedWarning = warned;
      drawPosition(direction, seconds, warned);
    }
    delay(100);
  }
}

void drawConfiguredLocation(Watchy &watch) {
  beginAppDisplay("SAVED LOCATION");
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(4, 42);
  if (watch.settings.lat.length() > 0 && watch.settings.lon.length() > 0) {
    Watchy::display.print("LAT ");
    Watchy::display.println(watch.settings.lat);
    Watchy::display.setCursor(4, 62);
    Watchy::display.print("LON ");
    Watchy::display.println(watch.settings.lon);
  } else if (watch.settings.cityID.length() > 0) {
    Watchy::display.print("WEATHER CITY ID ");
    Watchy::display.println(watch.settings.cityID);
    Watchy::display.setCursor(4, 62);
    Watchy::display.println("No coordinates configured");
  } else {
    Watchy::display.println("No saved location");
  }
  Watchy::display.setCursor(4, 108);
  Watchy::display.println("STATIC CONFIGURATION ONLY");
  Watchy::display.setCursor(4, 128);
  Watchy::display.println("WATCHY HAS NO GPS");
  Watchy::display.setCursor(4, 166);
  Watchy::display.println("Do not use as live position");
  finishAppDisplay();
}

void drawSosScreen(const EmergencyProfile::Data &profile) {
  beginAppDisplay("EMERGENCY");
  Watchy::display.setFont();
  Watchy::display.setTextSize(5);
  Watchy::display.setCursor(48, 70);
  Watchy::display.println("SOS");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(4, 116);
  Watchy::display.print("NAME  "); Watchy::display.println(profile.name);
  Watchy::display.setCursor(4, 133);
  Watchy::display.print("BLOOD "); Watchy::display.println(profile.bloodType);
  Watchy::display.setCursor(4, 150);
  Watchy::display.print("ICE   "); Watchy::display.println(profile.icePhone);
  Watchy::display.setCursor(4, 176);
  Watchy::display.println("CALL LOCAL EMERGENCY SERVICES");
  Watchy::display.setCursor(4, 190);
  Watchy::display.println("SELECT: REPEAT VIBRATION");
  finishAppDisplay();
}

void runSosScreen(Watchy &watch) {
  WatchyUi::Input::begin();
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
  drawSosScreen(profile);
  watch.vibMotor(120, 12);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    if (event == WatchyUi::Event::SELECT) {
      watch.vibMotor(120, 12);
    }
  }
}

void drawSosBeacon(bool active, const EmergencyProfile::Data &profile) {
  beginAppDisplay("SOS BLE BEACON");
  Watchy::display.setFont();
  Watchy::display.setTextSize(2);
  Watchy::display.setCursor(10, 60);
  Watchy::display.println(active ? "BROADCASTING" : "PAUSED");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(10, 102);
  Watchy::display.print("BLOOD "); Watchy::display.println(profile.bloodType);
  Watchy::display.setCursor(10, 120);
  Watchy::display.print("ICE   "); Watchy::display.println(profile.icePhone);
  Watchy::display.setCursor(10, 151);
  Watchy::display.println("Nearby BLE only. No EMS call.");
  Watchy::display.setCursor(10, 170);
  Watchy::display.println("SELECT: PAUSE  BACK: STOP");
  finishAppDisplay();
}

void runSosBeacon() {
  WatchyUi::Input::begin();
  EmergencyProfile::Data profile;
  EmergencyProfile::load(profile);
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
    if (event == WatchyUi::Event::BACK) {
      break;
    }
    if (event == WatchyUi::Event::SELECT) {
      active = !active;
      if (active) advertising->start();
      else advertising->stop();
      drawSosBeacon(active, profile);
    }
  }
  advertising->stop();
  BLEDevice::deinit(false);
  btStop();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

} // namespace

void Watchy::showSafetyTool(uint8_t tool) {
  switch (tool) {
  case FALL_DETECTOR: runFallDetector(*this); break;
  case BODY_POSITION: runPositionMonitor(*this); break;
  case CONFIGURED_LOCATION:
    drawConfiguredLocation(*this);
    return;
  case SOS_SCREEN: runSosScreen(*this); break;
  case SOS_BLE: runSosBeacon(); break;
  default: return;
  }
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
#include "demo/GalleryFixtures.h"

namespace WatchyDemo {
namespace {

void drawLocationPreview() {
  beginAppDisplay("SAVED LOCATION");
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(4, 42);
  Watchy::display.println("LAT 40.4168");
  Watchy::display.setCursor(4, 62);
  Watchy::display.println("LON -3.7038");
  Watchy::display.setCursor(4, 108);
  Watchy::display.println("STATIC CONFIGURATION ONLY");
  Watchy::display.setCursor(4, 128);
  Watchy::display.println("WATCHY HAS NO GPS");
  Watchy::display.setCursor(4, 166);
  Watchy::display.println("Do not use as live position");
  finishAppDisplay();
}

} // namespace

void renderSafetyPreview(uint8_t tool) {
  EmergencyProfile::Data profile = GalleryFixtures::emergencyProfile();
  switch (tool) {
  case FALL_DETECTOR: drawFallState(FALL_MONITORING); break;
  case BODY_POSITION: drawPosition(DIRECTION_DISP_UP, 0, false); break;
  case CONFIGURED_LOCATION: drawLocationPreview(); break;
  case SOS_SCREEN: drawSosScreen(profile); break;
  case SOS_BLE: drawSosBeacon(true, profile); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif