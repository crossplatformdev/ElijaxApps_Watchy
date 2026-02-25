#include "NetworkAppCommon.h"

void Watchy::showReverseDnsQuery() {
  String value;
  if (!NetworkApps::editText("REVERSE DNS", "1.1.1.1", value)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "REVERSE DNS")) {
    showMenu(menuIndex, false);
    return;
  }

  NetworkApps::TextDocument document;
  IPAddress address;
  if (!address.fromString(value)) {
    NetworkApps::addLine(document, "Invalid IPv4 address");
  } else {
    String query = String(address[3]) + "." + address[2] + "." + address[1] +
                   "." + address[0] + ".in-addr.arpa";
    NetworkApps::showStatus("REVERSE DNS", "Requesting PTR record...");
    if (!NetworkApps::dnsQuery(query, 12, document)) {
      NetworkApps::addLine(document, "DNS request failed");
    }
  }
  NetworkApps::disconnect();
  NetworkApps::viewDocument("REVERSE DNS", document);
  showMenu(menuIndex, false);
}