#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/NetUtils.h"
#include "../sdk/UiTemplates.h"
#include "NetAppCommon.h"
#include <WiFi.h>
#include <cstring>

// Port scanner built on lwIP sockets (ESP32 standard).
// - TCP Connect scan: uses non-blocking connect + select() timeout
// - UDP "probe" scan: reports OPEN only when a response is received

#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <fcntl.h>
#include <errno.h>

namespace {

static volatile bool sPortScanCancelRequested = false;

static void portScanBack(Watchy *watchy) {
  
  sPortScanCancelRequested = true;
}

enum class ScanMode : uint8_t {
  QuickTcp = 0,
  TcpTop1024,
  TcpFull,
  UdpQuick,
};

static const uint16_t kCommonTcpPorts[] = {
    21, 22, 23, 25, 53, 80, 110, 123, 143, 161, 389, 443, 445, 587, 993, 995, 1883, 3306, 3389, 5900, 8080,
};

static const uint16_t kCommonUdpPorts[] = {
    53, 67, 68, 69, 123, 161, 500, 1900,
};

static bool chooseScanMode(Watchy &watchy, ScanMode &outMode) {
  static UIMenuItemSpec items[] = {
      {"Quick TCP (common)"},
      {"TCP connect (1-1024)"},
      {"TCP connect (1-65535)"},
      {"Quick UDP probe"},
  };

  int8_t selected = static_cast<int8_t>(outMode);
  UiTemplates::MenuPickerLayout layout;
  layout.headerX = 16;
  layout.headerY = 36;
  layout.menuX = 0;
  layout.menuY = 72;
  layout.visibleRowsMax = 4;
  layout.startIndex = 0;
  layout.autoScroll = false;
  layout.font = &FreeMonoBold9pt7b;

  const int8_t chosen = UiTemplates::runMenuPicker(watchy,
                                                   "Scan Mode",
                                                   items,
                                                   static_cast<uint8_t>(sizeof(items) / sizeof(items[0])),
                                                   selected,
                                                   layout,
                                                   "BACK",
                                                   "UP",
                                                   "ACCEPT",
                                                   "DOWN");
  if (chosen < 0) {
    return false;
  }
  outMode = static_cast<ScanMode>(chosen);
  return true;
}

static bool resolveHost(const char *host, IPAddress &outIp) {
  if (host == nullptr || host[0] == '\0') {
    return false;
  }
  if (outIp.fromString(host)) {
    return true;
  }
  return WiFi.hostByName(host, outIp);
}

static void appendLine(String &out, const String &line, size_t maxLen) {
  if (out.length() >= maxLen) {
    return;
  }
  if (out.length() + line.length() + 1 > maxLen) {
    out += "\n[output truncated]";
    return;
  }
  out += line;
  out += "\n";
}

static bool tcpConnectPort(const IPAddress &ip, uint16_t port, uint32_t timeoutMs) {
  int sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    return false;
  }

  int flags = fcntl(sock, F_GETFL, 0);
  if (flags >= 0) {
    (void)fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = static_cast<uint32_t>(ip);

  const int res = ::connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  if (res == 0) {
    ::close(sock);
    return true;
  }

  if (errno != EINPROGRESS && errno != EALREADY) {
    ::close(sock);
    return false;
  }

  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(sock, &wfds);

  timeval tv{};
  tv.tv_sec = timeoutMs / 1000;
  tv.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000) * 1000);

  const int sel = ::select(sock + 1, nullptr, &wfds, nullptr, &tv);
  if (sel <= 0) {
    ::close(sock);
    return false;
  }

  int soErr = 0;
  socklen_t soErrLen = sizeof(soErr);
  if (::getsockopt(sock, SOL_SOCKET, SO_ERROR, &soErr, &soErrLen) != 0) {
    ::close(sock);
    return false;
  }

  ::close(sock);
  return (soErr == 0);
}

static bool udpProbePort(const IPAddress &ip, uint16_t port, uint32_t timeoutMs) {
  int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = static_cast<uint32_t>(ip);

  uint8_t b = 0;
  const int sent = ::sendto(sock, &b, 1, 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  if (sent < 0) {
    ::close(sock);
    return false;
  }

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(sock, &rfds);

  timeval tv{};
  tv.tv_sec = timeoutMs / 1000;
  tv.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000) * 1000);

  const int sel = ::select(sock + 1, &rfds, nullptr, nullptr, &tv);
  if (sel <= 0) {
    ::close(sock);
    return false;
  }

  char buf[32];
  const int recvd = ::recv(sock, buf, sizeof(buf), 0);
  ::close(sock);
  return (recvd > 0);
}

} // namespace

void Watchy::showPortScanner() {
  guiState = APP_STATE;
  static int8_t sHistorySelected = -1;

  while (true) {
    char host[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
    if (!NetAppCommon::pickTargetEditAndPersist(*this, "hist_ports", "Scan target", sHistorySelected, host, sizeof(host))) {
      showMenu(menuIndex);
      return;
    }

    ScanMode mode = ScanMode::QuickTcp;
    if (!chooseScanMode(*this, mode)) {
      showMenu(menuIndex);
      return;
    }

    // Output viewer.
    UIScrollableTextSpec scroll{};
    scroll.x = 0;
    scroll.y = 18;
    scroll.w = WatchyDisplay::WIDTH;
    scroll.h = WatchyDisplay::HEIGHT - 36;
    scroll.font = UiSDK::tinyMono6x8();
    scroll.fillBackground = false;
    scroll.firstLine = 0;
    scroll.lineHeight = 10;
    scroll.centered = false;
    scroll.wrapLongLines = true;
    scroll.wrapContinuationAlignRight = true;

    // Match TextBrowser-style visible line computation.
    {
      const int16_t padding = 6;
      const int16_t contentH = scroll.h - padding * 2;
      uint8_t visibleLines = (contentH > 0) ? static_cast<uint8_t>(contentH / scroll.lineHeight) : 1;
      if (visibleLines < 1) visibleLines = 1;
      scroll.maxLines = visibleLines;
    }

    UIAppSpec app{};
    app.scrollTexts = &scroll;
    app.scrollTextCount = 1;

    String body;
    body.reserve(4096);
    scroll.textRef = &body;

    if (!NetUtils::ensureWiFi(*this, 1, 10000)) {
      body = "WiFi not connected";
      uint16_t firstLine = 0;
      const UiTemplates::ViewerAction action =
          UiTemplates::runScrollableViewer(*this, app, scroll, firstLine, "EXIT", "UP", "TARGETS", "DOWN", true);
      if (action == UiTemplates::ViewerAction::Accept) {
        continue;
      }
      showMenu(menuIndex);
      return;
    }

    IPAddress ip;
    if (!resolveHost(host, ip)) {
      body = String("Could not resolve:\n") + host;
      uint16_t firstLine = 0;
      const UiTemplates::ViewerAction action =
          UiTemplates::runScrollableViewer(*this, app, scroll, firstLine, "EXIT", "UP", "TARGETS", "DOWN", true);
      if (action == UiTemplates::ViewerAction::Accept) {
        continue;
      }
      showMenu(menuIndex);
      return;
    }

    const size_t kMaxOutput = 48 * 1024;
    const uint32_t timeoutMs = 220;

    body = String("Port scan ") + host + "\nIP: " + ip.toString() + "\n\n";

    UiTemplates::renderScrollablePage(*this, app, "BACK", "-", "-", "-");

    // BACK cancels the scan. We pump input by rendering the controls row during
    // the scan loops; no GPIO polling.
    sPortScanCancelRequested = false;
    setButtonHandlers(portScanBack, nullptr, nullptr, nullptr);

    const UIControlsRowLayout scanControls[4] = {
        {"BACK", &Watchy::backPressed},
        {"-", nullptr},
        {"-", nullptr},
        {"-", nullptr},
    };

    auto cancelled = [&]() -> bool {
      UiSDK::renderControlsRow(*this, scanControls);
      return sPortScanCancelRequested;
    };

    uint32_t scanned = 0;
    uint32_t totalToScan = 0;
    uint16_t openCount = 0;

    if (mode == ScanMode::QuickTcp) {
      totalToScan = static_cast<uint32_t>(sizeof(kCommonTcpPorts) / sizeof(kCommonTcpPorts[0]));
      appendLine(body, "Mode: Quick TCP (common)", kMaxOutput);
      appendLine(body, "", kMaxOutput);

      for (uint32_t i = 0; i < totalToScan; ++i) {
        if (cancelled()) {
          appendLine(body, "\n[scan cancelled]", kMaxOutput);
          break;
        }

        const uint16_t port = kCommonTcpPorts[i];
        const bool open = tcpConnectPort(ip, port, timeoutMs);
        ++scanned;
        if (open) {
          ++openCount;
          appendLine(body, String("TCP ") + port + " OPEN", kMaxOutput);
        }

        if ((i % 5) == 0) {
          UiTemplates::renderScrollablePage(*this, app, "BACK", "-", "-", "-");
        }
      }
    } else if (mode == ScanMode::TcpTop1024 || mode == ScanMode::TcpFull) {
      const uint16_t startPort = 1;
      const uint16_t endPort = (mode == ScanMode::TcpTop1024) ? 1024 : 65535;
      totalToScan = static_cast<uint32_t>(endPort - startPort + 1);
      appendLine(body, String("Mode: TCP connect (") + startPort + "-" + endPort + ")", kMaxOutput);
      appendLine(body, "Tip: BACK cancels", kMaxOutput);
      appendLine(body, "", kMaxOutput);

      for (uint32_t p = startPort; p <= endPort; ++p) {
        if (cancelled()) {
          appendLine(body, "\n[scan cancelled]", kMaxOutput);
          break;
        }

        const bool open = tcpConnectPort(ip, static_cast<uint16_t>(p), timeoutMs);
        ++scanned;
        if (open) {
          ++openCount;
          appendLine(body, String("TCP ") + p + " OPEN", kMaxOutput);
        }

        if ((p % 64) == 0) {
          appendLine(body, String("...") + " scanned " + scanned + "/" + totalToScan, kMaxOutput);
          UiTemplates::renderScrollablePage(*this, app, "BACK", "-", "-", "-");
        }
      }
    } else {
      totalToScan = static_cast<uint32_t>(sizeof(kCommonUdpPorts) / sizeof(kCommonUdpPorts[0]));
      appendLine(body, "Mode: Quick UDP probe", kMaxOutput);
      appendLine(body, "Note: no ICMP, so results are limited.", kMaxOutput);
      appendLine(body, "", kMaxOutput);

      for (uint32_t i = 0; i < totalToScan; ++i) {
        if (cancelled()) {
          appendLine(body, "\n[scan cancelled]", kMaxOutput);
          break;
        }

        const uint16_t port = kCommonUdpPorts[i];
        const bool gotResponse = udpProbePort(ip, port, 350);
        ++scanned;

        if (gotResponse) {
          ++openCount;
          appendLine(body, String("UDP ") + port + " OPEN (response)", kMaxOutput);
        } else {
          appendLine(body, String("UDP ") + port + " open|filtered", kMaxOutput);
        }

        if ((i % 4) == 0) {
          UiTemplates::renderScrollablePage(*this, app, "BACK", "-", "-", "-");
        }
      }
    }

    appendLine(body, "", kMaxOutput);
    appendLine(body, String("Scanned: ") + scanned + "/" + totalToScan, kMaxOutput);
    appendLine(body, String("Open: ") + openCount, kMaxOutput);

    uint16_t firstLine = 0;
    clearButtonHandlers();
    const UiTemplates::ViewerAction action =
        UiTemplates::runScrollableViewer(*this, app, scroll, firstLine, "EXIT", "UP", "TARGETS", "DOWN", true);

    if (action == UiTemplates::ViewerAction::Accept) {
      continue;
    }

    showMenu(menuIndex);
    return;
  }
}
