#include "NetworkAppCommon.h"

void Watchy::showTraceroute() {
  String host;
  if (!NetworkApps::editText("TRACE HOST", "1.1.1.1", host)) {
    showMenu(menuIndex, false);
    return;
  }
  if (!NetworkApps::ensureConnected(*this, "TRACEROUTE")) {
    showMenu(menuIndex, false);
    return;
  }

  NetworkApps::TextDocument document;
  IPAddress target;
  NetworkApps::addLine(document, host);
  if (!NetworkApps::resolveIPv4(host, target)) {
    NetworkApps::addLine(document, "Host not found");
  } else {
    NetworkApps::prepareButtons();
    NetworkApps::showStatus("TRACEROUTE", "Tracing up to 12 hops...");
    for (uint8_t ttl = 1; ttl <= 12; ttl++) {
      if (NetworkApps::backPressed()) {
        while (NetworkApps::backPressed()) delay(10);
        NetworkApps::addLine(document, "Cancelled");
        break;
      }
      NetworkApps::EchoResult result =
          NetworkApps::sendEcho(target, ttl, ttl, 1000);
      String line = String(ttl) + "  ";
      if (result.status == NetworkApps::ECHO_TIMEOUT) {
        line += "*";
      } else if (result.status == NetworkApps::ECHO_ERROR) {
        line += "socket error";
      } else {
        line += result.responder.toString() + " " + result.elapsedMs + "ms";
      }
      NetworkApps::addLine(document, line);
      if (ttl % 3 == 0) {
        NetworkApps::drawDocumentPage("TRACEROUTE", document, 0);
      }
      if (result.status == NetworkApps::ECHO_REPLY ||
          result.status == NetworkApps::ECHO_UNREACHABLE ||
          result.status == NetworkApps::ECHO_ERROR) {
        break;
      }
    }
  }

  NetworkApps::disconnect();
  NetworkApps::viewDocument("TRACEROUTE", document);
  showMenu(menuIndex, false);
}