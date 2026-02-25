#ifndef WATCHY_BLUETOOTH_TOOL_APPS_H
#define WATCHY_BLUETOOTH_TOOL_APPS_H

#include <Arduino.h>

class BLEScanResults;
class Watchy;

namespace WatchyBluetoothTools {

void runScanner(BLEScanResults &results);
void runDeviceCount(BLEScanResults &results);
void runStrongest(BLEScanResults &results);
void runNamedDevices(BLEScanResults &results);
void runServiceUuids(BLEScanResults &results);
void runManufacturers(BLEScanResults &results);
void runRssiBands(BLEScanResults &results);
void runAddresses(BLEScanResults &results);
void runRadar(BLEScanResults &results);
void runTxPower(BLEScanResults &results);
void runIBeacons(BLEScanResults &results);

void runWatchyBeacon();
void runBatteryBeacon();
void runTimeBeacon();
void runStepBeacon();
void runNameBadge();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderScannerPreview(uint8_t view);
void renderDeviceCountPreview(uint8_t view);
void renderStrongestPreview(uint8_t view);
void renderNamedDevicesPreview(uint8_t view);
void renderServiceUuidsPreview(uint8_t view);
void renderManufacturersPreview(uint8_t view);
void renderRssiBandsPreview(uint8_t view);
void renderAddressesPreview(uint8_t view);
void renderRadarPreview(uint8_t view);
void renderTxPowerPreview(uint8_t view);
void renderIBeaconsPreview(uint8_t view);

void renderWatchyBeaconPreview(uint8_t view);
void renderBatteryBeaconPreview(uint8_t view);
void renderTimeBeaconPreview(uint8_t view);
void renderStepBeaconPreview(uint8_t view);
void renderNameBadgePreview(uint8_t view);
#endif

} // namespace WatchyBluetoothTools

#endif