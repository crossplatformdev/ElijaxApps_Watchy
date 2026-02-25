#include "NetworkAppCommon.h"

namespace {

struct PortDefinition {
  uint16_t port;
  const char *service;
};

constexpr PortDefinition commonPorts[] = {
    {21, "ftp"},       {22, "ssh"},       {23, "telnet"},
    {25, "smtp"},      {53, "dns"},       {80, "http"},
    {110, "pop3"},     {139, "netbios"},  {143, "imap"},
    {443, "https"},    {445, "smb"},      {587, "submission"},
    {993, "imaps"},    {995, "pop3s"},    {1433, "mssql"},
    {3306, "mysql"},   {3389, "rdp"},     {5432, "postgres"},
    {5900, "vnc"},     {8080, "http-alt"}, {8443, "https-alt"}};

} // namespace

void Watchy::showPortScanner() {
  if (!NetworkApps::ensureConnected(*this, "PORT SCANNER")) {
    showMenu(menuIndex, false);
    return;
  }
  String defaultTarget = WiFi.gatewayIP().toString();
  if (defaultTarget == "0.0.0.0") defaultTarget = "192.168.1.1";
  NetworkApps::disconnect();
  String host;
  if (!NetworkApps::editText("SCAN HOST", defaultTarget, host)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "PORT SCANNER")) {
    showMenu(menuIndex, false);
    return;
  }

  NetworkApps::TextDocument document;
  NetworkApps::addLine(document, "Authorized hosts only");
  NetworkApps::addLine(document, host);
  IPAddress target;
  if (!NetworkApps::resolveIPv4(host, target)) {
    NetworkApps::addLine(document, "Host not found");
  } else {
    NetworkApps::showStatus("PORT SCANNER", "Scanning common TCP ports...");
    NetworkApps::prepareButtons();
    uint8_t openCount = 0;
    for (const PortDefinition &definition : commonPorts) {
      if (NetworkApps::backPressed()) {
        while (NetworkApps::backPressed()) delay(10);
        NetworkApps::addLine(document, "Scan cancelled");
        break;
      }
      WiFiClient client;
      if (client.connect(target, definition.port, 300)) {
        NetworkApps::addLine(document, String(definition.port) + " open " +
                                           definition.service);
        openCount++;
      }
      client.stop();
    }
    if (openCount == 0) {
      NetworkApps::addLine(document, "No common ports open");
    }
  }

  NetworkApps::disconnect();
  NetworkApps::viewDocument("PORT SCANNER", document);
  showMenu(menuIndex, false);
}