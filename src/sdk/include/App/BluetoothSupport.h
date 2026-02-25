#ifndef WATCHY_BLUETOOTH_SUPPORT_H
#define WATCHY_BLUETOOTH_SUPPORT_H

#include <Arduino.h>
#include <string>

class BLEAdvertisementData;
class BLEScan;
class BLEScanResults;
class Watchy;

namespace WatchyBluetoothSupport {

enum class Tool : uint8_t {
  Scanner,
  DeviceCount,
  Strongest,
  NamedDevices,
  ServiceUuids,
  Manufacturers,
  RssiBands,
  Addresses,
  Radar,
  TxPower,
  IBeacons,
  Beacon,
  BatteryBeacon,
  TimeBeacon,
  StepBeacon,
  NameBadge,
  Count
};

constexpr uint8_t firstBeaconTool = static_cast<uint8_t>(Tool::Beacon);
constexpr uint8_t toolCount = static_cast<uint8_t>(Tool::Count);
constexpr uint8_t maximumScanDevices = 24;

enum class ScanOutcome : uint8_t {
  Completed,
  Cancelled,
  Failed
};

using BeaconDataConfigurer =
    void (*)(BLEAdvertisementData &data, const char *&description);

Tool normalizeTool(uint8_t tool);
bool isBeaconTool(Tool tool);
const char *title(Tool tool);

std::string shortened(const std::string &value, size_t length);
uint8_t signalStrength(int rssi);
void drawBluetoothRow(int16_t y, const std::string &name, int rssi,
                      bool emphasized = false);
void drawRadarFrame();
void drawRadarSignal(uint8_t index, int rssi);
void drawEmptyBluetooth(const char *detail);
void drawScanning(const char *title);

ScanOutcome scanNearby(const char *title, BLEScan *&scan,
                       BLEScanResults &results);
void stopScanRadio(BLEScan *scan);
int limitedCount(BLEScanResults &results);
void rankBySignal(BLEScanResults &results, int *indices, int count);

void appendUint32(std::string &data, uint32_t value);
void drawBeacon(const char *title, const char *description, bool active);
void runBeacon(const char *title,
               BeaconDataConfigurer configureData);

} // namespace WatchyBluetoothSupport

#endif