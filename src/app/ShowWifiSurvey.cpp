#include "NetworkAppCommon.h"

void Watchy::showWifiSurvey() {
  NetworkApps::prepareButtons();
  setRadioCpuFrequency();
  WiFi.mode(WIFI_STA);
  NetworkApps::showStatus("WIFI SURVEY", "Passive channel scan...");
  int networkCount = WiFi.scanNetworks(false, true, true, 300);
  NetworkApps::TextDocument document;
  if (networkCount <= 0) {
    NetworkApps::addLine(document, "No networks found");
  } else {
    NetworkApps::addLine(document, String(networkCount) + " networks");
    NetworkApps::addLine(document, "");
    for (int index = 0; index < networkCount; index++) {
      String security = WiFi.encryptionType(index) == WIFI_AUTH_OPEN
                            ? "open"
                            : "secured";
      NetworkApps::addLine(document, WiFi.SSID(index));
      NetworkApps::addLine(document, String(WiFi.RSSI(index)) + "dBm ch" +
                                         WiFi.channel(index) + " " + security);
      NetworkApps::addLine(document, WiFi.BSSIDstr(index));
    }
  }
  WiFi.scanDelete();
  NetworkApps::disconnect();
  NetworkApps::viewDocument("WIFI SURVEY", document);
  showMenu(menuIndex, false);
}