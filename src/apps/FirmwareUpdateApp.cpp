#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

// BLE OTA firmware update flow; moved from Watchy.cpp into its own OS app unit.
static void renderStatus(Watchy &watchy, bool partialRefresh,
                         const char *l1, const char *l2 = nullptr,
                         const char *l3 = nullptr, const char *l4 = nullptr,
                         const char *l5 = nullptr) {
  (void)partialRefresh;
  const char *lines[5] = {l1, l2, l3, l4, l5};
  UiTemplates::renderStatusLines(
  watchy,
      lines,
      5,
      /*x=*/0,
      /*y=*/30,
      /*lineSpacing=*/20,
      &FreeMonoBold9pt7b,
      "BACK");
}

static volatile bool sFwExitRequested = false;

static void fwBack(Watchy *watchy) {
  
  sFwExitRequested = true;
}

void Watchy::updateFWBegin() {
  setButtonHandlers(fwBack, nullptr, nullptr, nullptr);
  sFwExitRequested = false;

  UIControlsRowLayout controls[4] = {
      {"BACK", &Watchy::backPressed},
      {"", &Watchy::upPressed},
      {"", &Watchy::menuPressed},
      {"", &Watchy::downPressed},
  };

  renderStatus(*this,
               /*partialRefresh=*/false,
               "Bluetooth Started",
               " ",
               "Watchy BLE OTA",
               " ",
               "Waiting for connection...");

  BLE BT;
  BT.begin("Watchy BLE OTA");
  int prevStatus = -1;
  int currentStatus;

  // While BACK button is not pressed, keep checking for updates.
  while (!sFwExitRequested) {
    // Pump input frequently.
    UiSDK::renderControlsRow(*this, controls);

    currentStatus = BT.updateStatus();
    if (prevStatus != currentStatus || prevStatus == 1) {
      if (currentStatus == 0) {
        renderStatus(*this,
                     /*partialRefresh=*/false,
                     "BLE Connected!",
                     " ",
                     "Waiting for",
                     "upload...",
                     nullptr);
      }
      if (currentStatus == 1) {
        char bytesBuf[24];
        snprintf(bytesBuf, sizeof(bytesBuf), "%d bytes", BT.howManyBytes());
        renderStatus(*this,
                     /*partialRefresh=*/true,
                     "Downloading",
                     "firmware:",
                     " ",
                     bytesBuf,
                     nullptr);
      }
      if (currentStatus == 2) {
        renderStatus(*this,
                     /*partialRefresh=*/false,
                     "Download",
                     "completed!",
                     " ",
                     "Rebooting...",
                     nullptr);

        delay(2000);
        esp_restart();
      }
      if (currentStatus == 4) {
        renderStatus(*this,
                     /*partialRefresh=*/false,
                     "BLE Disconnected!",
                     " ",
                     "exiting...",
                     nullptr,
                     nullptr);
        delay(2000);
        break;
      }
      prevStatus = currentStatus;
    }
    delay(10);
  }
  clearButtonHandlers();
  delay(100);
  showMenu(menuIndex);
}

// Entry point from the menu to start the BLE OTA update flow.
void Watchy::showUpdateFW() {
  guiState = FW_UPDATE_STATE;
  updateFWBegin();
}
