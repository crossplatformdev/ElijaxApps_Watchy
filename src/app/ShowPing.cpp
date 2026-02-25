#include "NetworkAppCommon.h"

#include "WatchyUi.h"
#include "Watchy.h"

void showPingImpl(Watchy *watchy) {
  String host;
  if (!NetworkApps::editText("PING HOST", NETWORKING_PING_HOST, host)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected("PING", watchy)) {
    if (watchy != nullptr) watchy->showMenu(menuIndex, false);
    else WatchySdk::showMenu(menuIndex, false);
    return;
  }

  IPAddress address;
  NetworkApps::TextDocument document;
  NetworkApps::addLine(document, host);
  if (!NetworkApps::resolveIPv4(host, address)) {
    NetworkApps::addLine(document, "Host not found");
  } else {
    NetworkApps::addLine(document, address.toString());
    uint32_t totalTime = 0;
    uint8_t replies = 0;
    NetworkApps::showStatus("PING", "Sending ICMP echo...");
    for (uint8_t attempt = 1; attempt <= 4; attempt++) {
      NetworkApps::EchoResult result =
          NetworkApps::sendEcho(address, 64, attempt, 1200);
      if (result.status == NetworkApps::ECHO_REPLY) {
        NetworkApps::addLine(document, String(attempt) + ": " +
                                           result.responder.toString() + " " +
                                           result.elapsedMs + " ms");
        totalTime += result.elapsedMs;
        replies++;
      } else {
        NetworkApps::addLine(document, String(attempt) + ": timeout");
      }
    }
    NetworkApps::addLine(document, "");
    NetworkApps::addLine(document, String(replies) + "/4 replies");
    if (replies > 0) {
      NetworkApps::addLine(document,
                           "Average " + String(totalTime / replies) + " ms");
    }
  }

  NetworkApps::disconnect();
  NetworkApps::viewDocument("PING", document);
  if (watchy != nullptr) watchy->showMenu(menuIndex, false);
  else WatchySdk::showMenu(menuIndex, false);
}

void Watchy::showPing() { showPingImpl(this); }

void WatchySdk::showPing() { showPingImpl(nullptr); }
