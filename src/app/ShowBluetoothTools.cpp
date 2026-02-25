#include <Watchy.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <algorithm>
#include <string>
#include "AppDisplay.h"
#include "BatteryModel.h"

namespace {

enum BluetoothTool : uint8_t {
  BLE_SCANNER,
  BLE_DEVICE_COUNT,
  BLE_STRONGEST,
  BLE_NAMED,
  BLE_SERVICES,
  BLE_MANUFACTURERS,
  BLE_RSSI_BANDS,
  BLE_ADDRESSES,
  BLE_RADAR,
  BLE_TX_POWER,
  BLE_IBEACONS,
  BLE_BEACON,
  BLE_BATTERY_BEACON,
  BLE_TIME_BEACON,
  BLE_STEP_BEACON,
  BLE_NAME_BADGE,
  BLUETOOTH_TOOL_COUNT
};

constexpr uint8_t FIRST_BEACON_TOOL = BLE_BEACON;
constexpr uint8_t MAX_SCAN_DEVICES = 24;

const char *const titles[BLUETOOTH_TOOL_COUNT] = {
    "BLE SCANNER",  "DEVICE COUNT", "STRONGEST BLE", "NAMED DEVICES",
    "SERVICE UUIDS", "MANUFACTURERS", "RSSI BANDS",    "BLE ADDRESSES",
    "BLE RADAR",    "TX POWER",      "IBEACON WATCH", "WATCHY BEACON",
    "BATTERY BEACON", "TIME BEACON", "STEP BEACON",   "NAME BADGE"};

void useSmallText(int16_t x = 4, int16_t y = 38) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

std::string shortened(const std::string &value, size_t length) {
  if (value.length() <= length) {
    return value;
  }
  return value.substr(0, length);
}

void drawScanning(const char *title) {
  beginAppDisplay(title);
  useSmallText(22, 82);
  Watchy::display.setTextSize(2);
  Watchy::display.println("SCANNING");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(22, 125);
  Watchy::display.println("Active BLE scan: 3s");
  finishAppDisplay();
}

void stopScanRadio(BLEScan *scan) {
  if (scan != nullptr) {
    scan->stop();
    scan->clearResults();
  }
  BLEDevice::deinit(false);
  btStop();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

BLEScanResults scanNearby(const char *title, BLEScan *&scan) {
  drawScanning(title);
  Watchy::setRadioCpuFrequency();
  BLEDevice::init("");
  BLE_CONFIGURED = true;
  scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);
  return scan->start(3, false);
}

int limitedCount(BLEScanResults &results) {
  return min(results.getCount(), static_cast<int>(MAX_SCAN_DEVICES));
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

void drawScannerResults(BLEScanResults &results) {
  int count = limitedCount(results);
  int indices[MAX_SCAN_DEVICES];
  rankBySignal(results, indices, count);
  useSmallText(2, 34);
  Watchy::display.print("FOUND ");
  Watchy::display.println(results.getCount());
  for (int row = 0; row < min(count, 8); row++) {
    BLEAdvertisedDevice device = results.getDevice(indices[row]);
    std::string name = device.haveName() ? device.getName() : "(anonymous)";
    Watchy::display.setCursor(2, 53 + row * 17);
    Watchy::display.print(shortened(name, 19).c_str());
    Watchy::display.setCursor(154, 53 + row * 17);
    Watchy::display.print(device.getRSSI());
  }
}

void drawDeviceCount(BLEScanResults &results) {
  int named = 0;
  int services = 0;
  int manufacturers = 0;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    named += device.haveName();
    services += device.haveServiceUUID();
    manufacturers += device.haveManufacturerData();
  }
  useSmallText(12, 48);
  Watchy::display.setTextSize(4);
  Watchy::display.println(results.getCount());
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 104);
  Watchy::display.print("NAMED          "); Watchy::display.println(named);
  Watchy::display.setCursor(12, 128);
  Watchy::display.print("WITH SERVICE   "); Watchy::display.println(services);
  Watchy::display.setCursor(12, 152);
  Watchy::display.print("WITH MFR DATA  "); Watchy::display.println(manufacturers);
}

void drawStrongest(BLEScanResults &results) {
  if (results.getCount() == 0) {
    useSmallText(18, 85);
    Watchy::display.setTextSize(2);
    Watchy::display.println("NO DEVICES");
    return;
  }
  BLEAdvertisedDevice strongest = results.getDevice(0);
  for (int index = 1; index < results.getCount(); index++) {
    BLEAdvertisedDevice candidate = results.getDevice(index);
    if (candidate.getRSSI() > strongest.getRSSI()) {
      strongest = candidate;
    }
  }
  std::string name = strongest.haveName() ? strongest.getName() : "Anonymous";
  useSmallText(5, 48);
  Watchy::display.setTextSize(2);
  Watchy::display.println(shortened(name, 15).c_str());
  Watchy::display.setTextSize(4);
  Watchy::display.setCursor(32, 112);
  Watchy::display.print(strongest.getRSSI());
  Watchy::display.setTextSize(1);
  Watchy::display.println(" dBm");
  Watchy::display.setCursor(5, 150);
  Watchy::display.println(strongest.getAddress().toString().c_str());
}

void drawNamed(BLEScanResults &results) {
  useSmallText(2, 36);
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 9; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveName()) {
      continue;
    }
    Watchy::display.setCursor(2, 36 + row * 17);
    Watchy::display.print(shortened(device.getName(), 23).c_str());
    row++;
  }
  if (row == 0) {
    Watchy::display.println("No device names found");
  }
}

void drawServices(BLEScanResults &results) {
  useSmallText(2, 36);
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 8; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveServiceUUID()) {
      continue;
    }
    std::string uuid = device.getServiceUUID().toString();
    Watchy::display.setCursor(2, 36 + row * 19);
    Watchy::display.print(shortened(uuid, 31).c_str());
    row++;
  }
  if (row == 0) {
    Watchy::display.println("No advertised services");
  }
}

void drawManufacturers(BLEScanResults &results) {
  useSmallText(2, 36);
  int row = 0;
  for (int index = 0; index < results.getCount() && row < 8; index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveManufacturerData()) {
      continue;
    }
    std::string data = device.getManufacturerData();
    uint16_t company = data.size() >= 2
                           ? static_cast<uint8_t>(data[0]) |
                                 static_cast<uint16_t>(static_cast<uint8_t>(data[1])) << 8
                           : 0;
    Watchy::display.setCursor(2, 36 + row * 19);
    Watchy::display.print("ID 0x");
    if (company < 0x1000) Watchy::display.print('0');
    if (company < 0x100) Watchy::display.print('0');
    if (company < 0x10) Watchy::display.print('0');
    Watchy::display.print(company, HEX);
    Watchy::display.print("  ");
    Watchy::display.print(data.size());
    Watchy::display.println(" B");
    row++;
  }
  if (row == 0) {
    Watchy::display.println("No manufacturer data");
  }
}

void drawRssiBands(BLEScanResults &results) {
  int close = 0;
  int near = 0;
  int far = 0;
  for (int index = 0; index < results.getCount(); index++) {
    int rssi = results.getDevice(index).getRSSI();
    if (rssi >= -60) close++;
    else if (rssi >= -80) near++;
    else far++;
  }
  useSmallText(12, 54);
  Watchy::display.print("CLOSE  >= -60 dBm   "); Watchy::display.println(close);
  Watchy::display.setCursor(12, 91);
  Watchy::display.print("NEAR   -61..-80     "); Watchy::display.println(near);
  Watchy::display.setCursor(12, 128);
  Watchy::display.print("FAR    < -80 dBm    "); Watchy::display.println(far);
}

void drawAddresses(BLEScanResults &results) {
  useSmallText(2, 36);
  int count = min(results.getCount(), 8);
  for (int row = 0; row < count; row++) {
    BLEAdvertisedDevice device = results.getDevice(row);
    Watchy::display.setCursor(2, 36 + row * 19);
    Watchy::display.print(device.getAddress().toString().c_str());
    Watchy::display.print(' ');
    Watchy::display.println(device.getRSSI());
  }
  if (count == 0) {
    Watchy::display.println("No addresses found");
  }
}

void drawRadar(BLEScanResults &results) {
  int count = limitedCount(results);
  int indices[MAX_SCAN_DEVICES];
  rankBySignal(results, indices, count);
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  useSmallText(2, 38);
  for (int row = 0; row < min(count, 7); row++) {
    BLEAdvertisedDevice device = results.getDevice(indices[row]);
    int strength = constrain(device.getRSSI() + 100, 0, 70);
    Watchy::display.setCursor(2, 38 + row * 22);
    Watchy::display.print(device.getRSSI());
    Watchy::display.drawRect(46, 29 + row * 22, 146, 12, color);
    Watchy::display.fillRect(49, 32 + row * 22, strength * 2, 6, color);
  }
  if (count == 0) {
    Watchy::display.println("No signals found");
  }
}

void drawTxPower(BLEScanResults &results) {
  int count = 0;
  int total = 0;
  int minimum = 127;
  int maximum = -127;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (device.haveTXPower()) {
      int power = device.getTXPower();
      total += power;
      minimum = min(minimum, power);
      maximum = max(maximum, power);
      count++;
    }
  }
  useSmallText(12, 52);
  Watchy::display.print("REPORTING TX  "); Watchy::display.println(count);
  if (count > 0) {
    Watchy::display.setCursor(12, 86);
    Watchy::display.print("AVERAGE       "); Watchy::display.print(total / count); Watchy::display.println(" dBm");
    Watchy::display.setCursor(12, 120);
    Watchy::display.print("MIN / MAX     "); Watchy::display.print(minimum); Watchy::display.print(" / "); Watchy::display.println(maximum);
  }
}

void drawIBeacons(BLEScanResults &results) {
  int ibeacons = 0;
  int applePackets = 0;
  for (int index = 0; index < results.getCount(); index++) {
    BLEAdvertisedDevice device = results.getDevice(index);
    if (!device.haveManufacturerData()) continue;
    std::string data = device.getManufacturerData();
    bool apple = data.size() >= 2 && static_cast<uint8_t>(data[0]) == 0x4c &&
                 static_cast<uint8_t>(data[1]) == 0x00;
    applePackets += apple;
    ibeacons += apple && data.size() >= 4 &&
                static_cast<uint8_t>(data[2]) == 0x02 &&
                static_cast<uint8_t>(data[3]) == 0x15;
  }
  useSmallText(16, 58);
  Watchy::display.setTextSize(3);
  Watchy::display.println(ibeacons);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(16, 112);
  Watchy::display.println("iBeacon packets");
  Watchy::display.setCursor(16, 145);
  Watchy::display.print("Apple packets: ");
  Watchy::display.println(applePackets);
}

void drawScanResult(uint8_t tool, BLEScanResults &results) {
  beginAppDisplay(titles[tool]);
  switch (tool) {
  case BLE_SCANNER: drawScannerResults(results); break;
  case BLE_DEVICE_COUNT: drawDeviceCount(results); break;
  case BLE_STRONGEST: drawStrongest(results); break;
  case BLE_NAMED: drawNamed(results); break;
  case BLE_SERVICES: drawServices(results); break;
  case BLE_MANUFACTURERS: drawManufacturers(results); break;
  case BLE_RSSI_BANDS: drawRssiBands(results); break;
  case BLE_ADDRESSES: drawAddresses(results); break;
  case BLE_RADAR: drawRadar(results); break;
  case BLE_TX_POWER: drawTxPower(results); break;
  case BLE_IBEACONS: drawIBeacons(results); break;
  default: break;
  }
  finishAppDisplay();
}

void appendUint32(std::string &data, uint32_t value) {
  for (uint8_t byte = 0; byte < 4; byte++) {
    data.push_back(static_cast<char>((value >> (byte * 8)) & 0xff));
  }
}

void configureBeaconData(uint8_t tool, Watchy &watch,
                         BLEAdvertisementData &data,
                         const char *&description) {
  std::string payload;
  switch (tool) {
  case BLE_BEACON:
    data.setName("Watchy");
    payload.assign("WY", 2);
    payload.push_back(1);
    data.setManufacturerData(payload);
    description = "Broadcasting Watchy ID";
    break;
  case BLE_BATTERY_BEACON: {
    uint8_t percent =
        WatchyBattery::estimate(watch.getBatteryVoltage()).percent;
    data.setName("Watchy Battery");
    data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0x180F)));
    payload.push_back(static_cast<char>(percent));
    data.setServiceData(BLEUUID(static_cast<uint16_t>(0x180F)), payload);
    description = "Battery Service 0x180F";
    break;
  }
  case BLE_TIME_BEACON:
    watch.RTC.read(watch.currentTime);
    data.setName("Watchy Time");
    data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0x1805)));
    appendUint32(payload, static_cast<uint32_t>(currentUtcTime(watch.currentTime)));
    data.setServiceData(BLEUUID(static_cast<uint16_t>(0x1805)), payload);
    description = "UTC epoch service data";
    break;
  case BLE_STEP_BEACON:
    data.setName("Watchy Steps");
    data.setCompleteServices(BLEUUID(static_cast<uint16_t>(0xFFF0)));
    appendUint32(payload, sensor.getCounter());
    data.setServiceData(BLEUUID(static_cast<uint16_t>(0xFFF0)), payload);
    description = "Step count on 0xFFF0";
    break;
  case BLE_NAME_BADGE:
    data.setName("WATCHY BADGE");
    description = "Advertising device name";
    break;
  default:
    description = "Watchy BLE beacon";
    break;
  }
}

void drawBeacon(const char *title, const char *description, bool active) {
  beginAppDisplay(title);
  useSmallText(8, 54);
  Watchy::display.setTextSize(2);
  Watchy::display.println(active ? "ON AIR" : "PAUSED");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 105);
  Watchy::display.println(description);
  Watchy::display.setCursor(8, 155);
  Watchy::display.println("SELECT: PAUSE / RESUME");
  Watchy::display.setCursor(8, 178);
  Watchy::display.println("BACK: STOP AND EXIT");
  finishAppDisplay();
}

void runBeacon(uint8_t tool, Watchy &watch) {
  WatchyUi::Input::begin();
  Watchy::setRadioCpuFrequency();
  BLEDevice::init(titles[tool]);
  BLE_CONFIGURED = true;

  BLEAdvertisementData data;
  const char *description = nullptr;
  configureBeaconData(tool, watch, data, description);
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  advertising->setScanResponse(false);
  advertising->setAdvertisementData(data);
  advertising->start();
  bool active = true;
  drawBeacon(titles[tool], description, active);

  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      break;
    }
    if (event == WatchyUi::Event::SELECT) {
      active = !active;
      if (active) advertising->start();
      else advertising->stop();
      drawBeacon(titles[tool], description, active);
    }
  }
  advertising->stop();
  BLEDevice::deinit(false);
  btStop();
  BLE_CONFIGURED = false;
  Watchy::setLowPowerCpuFrequency();
}

} // namespace

void Watchy::showBluetoothTool(uint8_t tool) {
  if (tool >= BLUETOOTH_TOOL_COUNT) {
    tool = BLE_SCANNER;
  }
  if (tool >= FIRST_BEACON_TOOL) {
    runBeacon(tool, *this);
    showMenu(menuIndex, false);
    return;
  }

  BLEScan *scan = nullptr;
  BLEScanResults results = scanNearby(titles[tool], scan);
  drawScanResult(tool, results);
  stopScanRadio(scan);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {
namespace {

void drawDemoScanner() {
  useSmallText(2, 34);
  Watchy::display.println("FOUND 7");
  const char *const names[] = {
      "Watchy Demo", "Heart Sensor", "Keyboard", "(anonymous)",
      "Headphones", "Beacon Lab", "Tablet"};
  const int rssis[] = {-48, -57, -63, -71, -76, -82, -88};
  for (uint8_t row = 0; row < 7; row++) {
    Watchy::display.setCursor(2, 53 + row * 17);
    Watchy::display.print(names[row]);
    Watchy::display.setCursor(154, 53 + row * 17);
    Watchy::display.print(rssis[row]);
  }
}

void drawDemoCount() {
  useSmallText(12, 48);
  Watchy::display.setTextSize(4);
  Watchy::display.println(7);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(12, 104);
  Watchy::display.println("NAMED          6");
  Watchy::display.setCursor(12, 128);
  Watchy::display.println("WITH SERVICE   4");
  Watchy::display.setCursor(12, 152);
  Watchy::display.println("WITH MFR DATA  3");
}

void drawDemoStrongest() {
  useSmallText(5, 48);
  Watchy::display.setTextSize(2);
  Watchy::display.println("Watchy Demo");
  Watchy::display.setTextSize(4);
  Watchy::display.setCursor(32, 112);
  Watchy::display.print(-48);
  Watchy::display.setTextSize(1);
  Watchy::display.println(" dBm");
  Watchy::display.setCursor(5, 150);
  Watchy::display.println("02:00:00:00:00:01");
}

void drawDemoLines(const char *const lines[], uint8_t count,
                   int16_t firstY = 36, int16_t spacing = 19) {
  useSmallText(2, firstY);
  for (uint8_t row = 0; row < count; row++) {
    Watchy::display.setCursor(2, firstY + row * spacing);
    Watchy::display.println(lines[row]);
  }
}

void drawDemoRadar() {
  const int rssis[] = {-48, -57, -63, -71, -76, -82, -88};
  uint16_t color = WatchyUi::Theme::foreground();
  useSmallText(2, 38);
  for (uint8_t row = 0; row < 7; row++) {
    int strength = constrain(rssis[row] + 100, 0, 70);
    Watchy::display.setCursor(2, 38 + row * 22);
    Watchy::display.print(rssis[row]);
    Watchy::display.drawRect(46, 29 + row * 22, 146, 12, color);
    Watchy::display.fillRect(49, 32 + row * 22, strength * 2, 6, color);
  }
}

} // namespace

void renderBluetoothPreview(uint8_t tool) {
  if (tool >= BLUETOOTH_TOOL_COUNT) {
    tool = BLE_SCANNER;
  }
  if (tool >= FIRST_BEACON_TOOL) {
    const char *const descriptions[] = {
        "Broadcasting Watchy ID", "Battery Service 0x180F",
        "UTC epoch service data", "Step count on 0xFFF0",
        "Advertising device name"};
    drawBeacon(titles[tool], descriptions[tool - FIRST_BEACON_TOOL], true);
    return;
  }

  beginAppDisplay(titles[tool]);
  switch (tool) {
  case BLE_SCANNER:
    drawDemoScanner();
    break;
  case BLE_DEVICE_COUNT:
    drawDemoCount();
    break;
  case BLE_STRONGEST:
    drawDemoStrongest();
    break;
  case BLE_NAMED: {
    const char *const lines[] = {
        "Watchy Demo", "Heart Sensor", "Keyboard", "Headphones",
        "Beacon Lab", "Tablet"};
    drawDemoLines(lines, 6, 36, 17);
    break;
  }
  case BLE_SERVICES: {
    const char *const lines[] = {
        "0000180f-0000-1000-8000-00805f9b34fb",
        "0000180d-0000-1000-8000-00805f9b34fb",
        "00001812-0000-1000-8000-00805f9b34fb",
        "0000fff0-0000-1000-8000-00805f9b34fb"};
    drawDemoLines(lines, 4);
    break;
  }
  case BLE_MANUFACTURERS: {
    const char *const lines[] = {
        "ID 0x004C  27 B", "ID 0x0006  18 B", "ID 0x0131  12 B"};
    drawDemoLines(lines, 3);
    break;
  }
  case BLE_RSSI_BANDS: {
    const char *const lines[] = {
        "CLOSE  >= -60 dBm   2", "NEAR   -61..-80     3",
        "FAR    < -80 dBm    2"};
    drawDemoLines(lines, 3, 54, 37);
    break;
  }
  case BLE_ADDRESSES: {
    const char *const lines[] = {
        "02:00:00:00:00:01 -48", "02:00:00:00:00:02 -57",
        "02:00:00:00:00:03 -63", "02:00:00:00:00:04 -71",
        "02:00:00:00:00:05 -76", "02:00:00:00:00:06 -82"};
    drawDemoLines(lines, 6);
    break;
  }
  case BLE_RADAR:
    drawDemoRadar();
    break;
  case BLE_TX_POWER: {
    const char *const lines[] = {
        "REPORTING TX  4", "AVERAGE       -4 dBm",
        "MIN / MAX     -12 / 4"};
    drawDemoLines(lines, 3, 52, 34);
    break;
  }
  case BLE_IBEACONS:
    useSmallText(16, 58);
    Watchy::display.setTextSize(3);
    Watchy::display.println(2);
    Watchy::display.setTextSize(1);
    Watchy::display.setCursor(16, 112);
    Watchy::display.println("iBeacon packets");
    Watchy::display.setCursor(16, 145);
    Watchy::display.println("Apple packets: 3");
    break;
  default:
    break;
  }
  finishAppDisplay();
}

} // namespace WatchyDemo
#endif