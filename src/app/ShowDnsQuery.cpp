#include "NetworkAppCommon.h"

void Watchy::showDnsQuery() {
  String host;
  if (!NetworkApps::editText("DNS NAME", "example.com", host)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "DNS QUERY")) {
    showMenu(menuIndex, false);
    return;
  }

  NetworkApps::showStatus("DNS QUERY", "Requesting A records...");
  NetworkApps::TextDocument document;
  if (!NetworkApps::dnsQuery(host, 1, document)) {
    NetworkApps::addLine(document, "DNS request failed");
  }
  NetworkApps::disconnect();
  NetworkApps::viewDocument("DNS QUERY", document);
  showMenu(menuIndex, false);
}