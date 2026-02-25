#include "NetworkAppCommon.h"

#include "WatchyUi.h"
#include "Watchy.h"

void showReverseDnsQueryImpl(Watchy *watchy) {
  String value;
  if (!NetworkApps::editText("REVERSE DNS",
                             NETWORKING_REVERSE_DNS_ADDRESS, value)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected("REVERSE DNS", watchy)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
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
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showReverseDnsQuery() { showReverseDnsQueryImpl(this); }

void WatchySdk::showReverseDnsQuery() { showReverseDnsQueryImpl(nullptr); }
