#include "WatchyUi.h"
#include "Watchy.h"
#include <BLEScan.h>

#include "AppDisplay.h"
#include "BluetoothSupport.h"
#include "BluetoothToolApps.h"

namespace {

void renderScanResult(WatchyBluetoothSupport::Tool tool,
                      BLEScanResults &results) {
  using namespace WatchyBluetoothSupport;
  using namespace WatchyBluetoothTools;
  beginAppDisplay(title(tool));
  switch (tool) {
  case Tool::Scanner: runScanner(results); break;
  case Tool::DeviceCount: runDeviceCount(results); break;
  case Tool::Strongest: runStrongest(results); break;
  case Tool::NamedDevices: runNamedDevices(results); break;
  case Tool::ServiceUuids: runServiceUuids(results); break;
  case Tool::Manufacturers: runManufacturers(results); break;
  case Tool::RssiBands: runRssiBands(results); break;
  case Tool::Addresses: runAddresses(results); break;
  case Tool::Radar: runRadar(results); break;
  case Tool::TxPower: runTxPower(results); break;
  case Tool::IBeacons: runIBeacons(results); break;
  default: break;
  }
  finishAppDisplay();
}

void runBeaconTool(WatchyBluetoothSupport::Tool tool) {
  using namespace WatchyBluetoothSupport;
  using namespace WatchyBluetoothTools;
  switch (tool) {
  case Tool::Beacon: runWatchyBeacon(); break;
  case Tool::BatteryBeacon: runBatteryBeacon(); break;
  case Tool::TimeBeacon: runTimeBeacon(); break;
  case Tool::StepBeacon: runStepBeacon(); break;
  case Tool::NameBadge: runNameBadge(); break;
  default: break;
  }
}

} // namespace

void showBluetoothToolImpl(uint8_t rawTool, Watchy *watchy) {
  using namespace WatchyBluetoothSupport;
  Tool tool = normalizeTool(rawTool);
  if (isBeaconTool(tool)) {
    runBeaconTool(tool);
    if (watchy != nullptr) {
      watchy->showMenu(menuIndex, false);
    } else {
      WatchySdk::showMenu(menuIndex, false);
    }
    return;
  }
  BLEScan *scan = nullptr;
  BLEScanResults results;
  ScanOutcome outcome = scanNearby(title(tool), scan, results);
  if (outcome != ScanOutcome::Completed) {
    stopScanRadio(scan);
    if (outcome == ScanOutcome::Cancelled) {
      if (watchy != nullptr) {
        watchy->showMenu(menuIndex, false);
      } else {
        WatchySdk::showMenu(menuIndex, false);
      }
    } else {
      WatchyUi::Feedback::showMessage(title(tool), "BLE scan failed or timed out.",
                                      WatchyUi::MessageKind::ERROR, "BACK EXIT");
    }
    return;
  }
  renderScanResult(tool, results);
  stopScanRadio(scan);
}

void Watchy::showBluetoothTool(uint8_t rawTool) {
  showBluetoothToolImpl(rawTool, this);
}

void WatchySdk::showBluetoothTool(uint8_t rawTool) {
  showBluetoothToolImpl(rawTool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderBluetoothPreview(uint8_t rawTool, uint8_t view) {
  using namespace WatchyBluetoothSupport;
  using namespace WatchyBluetoothTools;
  Tool tool = normalizeTool(rawTool);
  if (isBeaconTool(tool)) {
    switch (tool) {
    case Tool::Beacon: renderWatchyBeaconPreview(view); break;
    case Tool::BatteryBeacon: renderBatteryBeaconPreview(view); break;
    case Tool::TimeBeacon: renderTimeBeaconPreview(view); break;
    case Tool::StepBeacon: renderStepBeaconPreview(view); break;
    case Tool::NameBadge: renderNameBadgePreview(view); break;
    default: break;
    }
    return;
  }
  if (view == 0) {
    drawScanning(title(tool));
    return;
  }
  beginAppDisplay(title(tool));
  if (view >= 2) {
    drawEmptyBluetooth("Scan completed with no signals");
  } else {
    switch (tool) {
    case Tool::Scanner: renderScannerPreview(view); break;
    case Tool::DeviceCount: renderDeviceCountPreview(view); break;
    case Tool::Strongest: renderStrongestPreview(view); break;
    case Tool::NamedDevices: renderNamedDevicesPreview(view); break;
    case Tool::ServiceUuids: renderServiceUuidsPreview(view); break;
    case Tool::Manufacturers: renderManufacturersPreview(view); break;
    case Tool::RssiBands: renderRssiBandsPreview(view); break;
    case Tool::Addresses: renderAddressesPreview(view); break;
    case Tool::Radar: renderRadarPreview(view); break;
    case Tool::TxPower: renderTxPowerPreview(view); break;
    case Tool::IBeacons: renderIBeaconsPreview(view); break;
    default: break;
    }
  }
  finishAppDisplay();
}

} // namespace WatchyDemo
#endif
