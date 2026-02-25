#include "WatchyUi.h"
#include "Watchy.h"
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <algorithm>
#include <freertos/task.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"
#include "WatchyPowerDiagnostics.h"

namespace WatchyBluetoothSupport {
namespace {

constexpr uint32_t scanDurationSeconds = 3;
constexpr uint32_t scanDeadlineMs = 5000;

portMUX_TYPE scanStateMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t scanWaitingTask = nullptr;
bool scanFinished = false;

const char *const titles[toolCount] = {
    "BLE SCANNER",  "DEVICE COUNT", "STRONGEST BLE", "NAMED DEVICES",
    "SERVICE UUIDS", "MANUFACTURERS", "RSSI BANDS",    "BLE ADDRESSES",
    "BLE RADAR",    "TX POWER",      "IBEACON WATCH", "WATCHY BEACON",
    "BATTERY BEACON", "TIME BEACON", "STEP BEACON",   "NAME BADGE"};

void scanComplete(BLEScanResults) {
  portENTER_CRITICAL(&scanStateMux);
  scanFinished = true;
  TaskHandle_t waitingTask = scanWaitingTask;
  portEXIT_CRITICAL(&scanStateMux);
  if (waitingTask != nullptr) xTaskNotifyGive(waitingTask);
}

void beginScanWait() {
  portENTER_CRITICAL(&scanStateMux);
  scanFinished = false;
  scanWaitingTask = xTaskGetCurrentTaskHandle();
  portEXIT_CRITICAL(&scanStateMux);
}

bool scanHasFinished() {
  portENTER_CRITICAL(&scanStateMux);
  bool finished = scanFinished;
  portEXIT_CRITICAL(&scanStateMux);
  return finished;
}

void endScanWait() {
  portENTER_CRITICAL(&scanStateMux);
  scanWaitingTask = nullptr;
  portEXIT_CRITICAL(&scanStateMux);
}

} // namespace

Tool normalizeTool(uint8_t tool) {
  return tool < toolCount ? static_cast<Tool>(tool) : Tool::Scanner;
}

bool isBeaconTool(Tool tool) {
  uint8_t index = static_cast<uint8_t>(tool);
  return index >= firstBeaconTool && index < toolCount;
}

const char *title(Tool tool) {
  uint8_t index = static_cast<uint8_t>(tool);
  return titles[index < toolCount ? index : 0];
}

std::string shortened(const std::string &value, size_t length) {
  if (value.length() <= length) {
    return value;
  }
  return value.substr(0, length);
}

uint8_t signalStrength(int rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -65) return 3;
  if (rssi >= -78) return 2;
  if (rssi >= -90) return 1;
  return 0;
}

void drawBluetoothRow(int16_t y, const std::string &name, int rssi,
                      bool emphasized) {
  if (emphasized) {
    WatchyUi::GrayPaint::fillRoundRect(
        {8, static_cast<int16_t>(y - 12), 184, 17}, 3,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  }
  const int16_t rowCenterY = static_cast<int16_t>(y - 12 + 17 / 2);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  std::string shortName = shortened(name, 13);
  std::string rssiText = std::to_string(rssi);
    Watchy::display.setCursor(
      12, AppVisual::centeredCursorY(rowCenterY, shortName.c_str()));
    Watchy::display.print(shortName.c_str());
    Watchy::display.setCursor(
      103, AppVisual::centeredCursorY(rowCenterY, rssiText.c_str()));
    Watchy::display.print(rssiText.c_str());
  AppVisual::drawSignalBars({142, static_cast<int16_t>(y - 12), 42, 15},
                            signalStrength(rssi));
}

void drawRadarFrame() {
  constexpr int16_t centerX = 100;
  constexpr int16_t centerY = 98;
  const uint16_t foreground = WatchyUi::Theme::foreground();
  for (int16_t radius = 18; radius <= 54; radius += 18) {
    Watchy::display.drawCircle(centerX, centerY, radius, foreground);
  }
  Watchy::display.drawLine(centerX - 54, centerY, centerX + 54, centerY,
                           foreground);
  Watchy::display.drawLine(centerX, centerY - 54, centerX, centerY + 54,
                           foreground);
  AppVisual::drawStatusIcon({84, 82, 32, 32}, AppVisual::StatusIcon::RADIO);
}

void drawRadarSignal(uint8_t index, int rssi) {
  constexpr int16_t centerX = 100;
  constexpr int16_t centerY = 98;
  static const int8_t directionX[] = {0, 55, 55, 0, -55, -55, 30};
  static const int8_t directionY[] = {-64, -35, 35, 62, 35, -35, -55};
  uint8_t direction = index % (sizeof(directionX) / sizeof(directionX[0]));
  int16_t distance = 14 + (4 - signalStrength(rssi)) * 10;
  int16_t x = centerX + directionX[direction] * distance / 64;
  int16_t y = centerY + directionY[direction] * distance / 64;
  Watchy::display.fillCircle(x, y, signalStrength(rssi) >= 3 ? 4 : 3,
                            WatchyUi::Theme::foreground());
}

void drawEmptyBluetooth(const char *detail) {
  AppVisual::drawEmptyState({8, 40, 184, 118}, "NO DEVICES", detail);
  WatchyUi::Widget::footer("BACK EXIT");
}

void drawScanning(const char *scanTitle) {
  beginAppDisplay(scanTitle);
  drawRadarFrame();
  WatchyUi::Canvas::centeredText({0, 159, 200, 18}, "SCANNING", 2,
                                 WatchyUi::Theme::foreground());
  WatchyUi::Widget::footer("ACTIVE BLE SCAN / 3 SECONDS  BACK CANCEL");
  finishAppDisplay();
}

void stopScanRadio(BLEScan *scan) {
  if (scan != nullptr) {
    scan->stop();
    scan->clearResults();
  }
  BLEDevice::deinit(false);
  btStop();
  WatchyDiagnostics::endBleSession();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

ScanOutcome scanNearby(const char *scanTitle, BLEScan *&scan,
                       BLEScanResults &results) {
  WatchyUi::Input::begin();
  drawScanning(scanTitle);
  WatchyDiagnostics::beginBleSession();
  Watchy::setRadioCpuFrequency();
  BLEDevice::init("");
  BLE_CONFIGURED = true;
  scan = BLEDevice::getScan();
  if (scan == nullptr) return ScanOutcome::Failed;
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);
  beginScanWait();
  if (!scan->start(scanDurationSeconds, scanComplete, false)) {
    endScanWait();
    return ScanOutcome::Failed;
  }

  ScanOutcome outcome = ScanOutcome::Completed;
  uint32_t startedAt = millis();
  while (!scanHasFinished()) {
    uint32_t elapsed = millis() - startedAt;
    if (elapsed >= scanDeadlineMs) {
      outcome = ScanOutcome::Failed;
      break;
    }
    WatchyUi::Event event = WatchyUi::Input::waitNotified(
        scanDeadlineMs - elapsed);
    if (event == WatchyUi::Event::BACK) {
      outcome = ScanOutcome::Cancelled;
      break;
    }
  }
  endScanWait();
  if (outcome != ScanOutcome::Completed) scan->stop();
  results = scan->getResults();
  return outcome;
}

int limitedCount(BLEScanResults &results) {
  return min(results.getCount(), static_cast<int>(maximumScanDevices));
}

void rankBySignal(BLEScanResults &results, int *indices, int count) {
  for (int index = 0; index < count; index++) {
    indices[index] = index;
  }
  for (int first = 0; first < count; first++) {
    for (int second = first + 1; second < count; second++) {
      if (results.getDevice(indices[second]).getRSSI() >
          results.getDevice(indices[first]).getRSSI()) {
        std::swap(indices[first], indices[second]);
      }
    }
  }
}

void appendUint32(std::string &data, uint32_t value) {
  for (uint8_t byte = 0; byte < 4; byte++) {
    data.push_back(static_cast<char>((value >> (byte * 8)) & 0xff));
  }
}

void drawBeacon(const char *beaconTitle, const char *description, bool active) {
  beginAppDisplay(beaconTitle);
  AppVisual::drawStatusIcon({79, 37, 42, 42},
                            active ? AppVisual::StatusIcon::RADIO
                                   : AppVisual::StatusIcon::EMPTY,
                            true);
  WatchyUi::Canvas::centeredText({0, 91, 200, 20}, active ? "ON AIR" : "PAUSED",
                                 2, WatchyUi::Theme::foreground());
  AppVisual::drawDataRow(142, "PAYLOAD", description, true);
  WatchyUi::Widget::footer("SELECT PAUSE / RESUME  BACK STOP");
  finishAppDisplay();
}

void runBeacon(const char *beaconTitle,
               BeaconDataConfigurer configureData) {
  WatchyUi::Input::begin();
  WatchyDiagnostics::beginBleSession();
  Watchy::setRadioCpuFrequency();
  BLEDevice::init(beaconTitle);
  BLE_CONFIGURED = true;

  BLEAdvertisementData data;
  const char *description = nullptr;
  configureData(data, description);
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  advertising->setScanResponse(false);
  advertising->setAdvertisementData(data);
  advertising->start();
  bool active = true;
  drawBeacon(beaconTitle, description, active);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      break;
    }
    if (event == WatchyUi::Event::MENU) {
      active = !active;
      if (active) advertising->start();
      else advertising->stop();
      drawBeacon(beaconTitle, description, active);
    }
  }
  advertising->stop();
  BLEDevice::deinit(false);
  btStop();
  WatchyDiagnostics::endBleSession();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

} // namespace WatchyBluetoothSupport
