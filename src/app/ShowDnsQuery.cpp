#include "WatchyUi.h"
#include "NetworkAppCommon.h"
#include "Watchy.h"

void showDnsQueryImpl(Watchy *watchy) {
  String host;
  if (!NetworkApps::editText("DNS NAME", NETWORKING_DNS_QUERY_NAME, host)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected("DNS QUERY", watchy)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }

  NetworkApps::showStatus("DNS QUERY", "Requesting A records...");
  NetworkApps::TextDocument document;
  if (!NetworkApps::dnsQuery(host, 1, document)) {
    NetworkApps::addLine(document, "DNS request failed");
  }
  NetworkApps::disconnect();
  NetworkApps::viewDocument("DNS QUERY", document);
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showDnsQuery() { showDnsQueryImpl(this); }

void WatchySdk::showDnsQuery() { showDnsQueryImpl(nullptr); }
