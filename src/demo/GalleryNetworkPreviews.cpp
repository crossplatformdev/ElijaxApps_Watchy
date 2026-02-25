#include "GalleryRenderers.h"

#ifdef WATCHY_DETERMINISTIC_GALLERY

#include "app/NetworkAppCommon.h"

namespace WatchyDemo {
namespace {

const char *const titles[] = {
    "BROWSER", "RSS FEED", "PING", "TRACEROUTE", "PORT SCANNER",
    "DNS QUERY", "REVERSE DNS", "DUCKDUCKGO", "WIFI SURVEY"};

void addLines(NetworkApps::TextDocument &document,
              const char *const lines[], uint8_t count) {
  for (uint8_t index = 0; index < count; index++) {
    NetworkApps::addLine(document, lines[index]);
  }
}

} // namespace

void renderNetworkPreview(uint8_t tool) {
  if (tool >= sizeof(titles) / sizeof(titles[0])) {
    tool = 0;
  }

  NetworkApps::TextDocument document;
  switch (tool) {
  case 0: {
    const char *const lines[] = {
        "https://example.com/watchy", "", "WATCHY DEMO",
        "A compact public preview", "rendered on the 200x200", "e-paper UI.",
        "", "[1] SDK documentation", "[2] Application gallery"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 1: {
    const char *const lines[] = {
        "Watchy Project News", "", "Deterministic gallery ready",
        "142 applications captured", "from the real framebuffer.", "",
        "Battery model calibrated", "Full charge: 3.95 V / 150 mAh"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 2: {
    const char *const lines[] = {
        "example.net", "192.0.2.1", "1: 192.0.2.1 18 ms",
        "2: 192.0.2.1 17 ms", "3: 192.0.2.1 19 ms",
        "4: 192.0.2.1 18 ms", "", "4/4 replies", "Average 18 ms"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 3: {
    const char *const lines[] = {
        "example.net [192.0.2.80]", "1  192.0.2.1       2 ms",
        "2  198.51.100.1   11 ms", "3  203.0.113.9    17 ms",
        "4  192.0.2.80     19 ms", "", "Trace complete"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 4: {
    const char *const lines[] = {
        "Authorized hosts only", "192.0.2.80", "22 open ssh",
        "80 open http", "443 open https", "", "3 common ports open"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 5: {
    const char *const lines[] = {
        "example.com A", "", "A 192.0.2.80", "TTL 300 seconds",
        "", "Status: NOERROR"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 6: {
    const char *const lines[] = {
        "192.0.2.42", "", "PTR watchy-demo.example.",
        "TTL 300 seconds", "", "Status: NOERROR"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 7: {
    const char *const lines[] = {
        "watchy open source", "", "Watchy documentation",
        "example.com/watchy/docs", "", "Watchy application gallery",
        "example.com/watchy/gallery"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  case 8: {
    const char *const lines[] = {
        "3 networks", "", "WATCHY_DEMO", "-47dBm ch6 secured",
        "02:00:00:00:00:01", "LAB_GUEST", "-68dBm ch11 secured",
        "02:00:00:00:00:02", "OPEN_DEMO", "-81dBm ch1 open",
        "02:00:00:00:00:03"};
    addLines(document, lines, sizeof(lines) / sizeof(lines[0]));
    break;
  }
  default:
    break;
  }
  NetworkApps::drawDocumentPage(titles[tool], document, 0);
}

} // namespace WatchyDemo

#endif