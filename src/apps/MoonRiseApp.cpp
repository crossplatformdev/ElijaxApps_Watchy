#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include <MoonRise.h>

namespace {

static void buildMoonRiseText(Watchy &watchy, void * /*userData*/, String &body) {
  Watchy::RTC.read(watchy.currentTime);
  time_t nowTs = makeTime(watchy.currentTime);
  double lat = watchy.settings.lat.length() ? watchy.settings.lat.toFloat() : 0.0;
  double lon = watchy.settings.lon.length() ? watchy.settings.lon.toFloat() : 0.0;

  MoonRise mr;
  mr.calculate(lat, lon, nowTs);

  body.reserve(192);
  body += "Moonrise: ";

  tmElements_t t;
  breakTime(mr.riseTime + gmtOffset, t);
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", t.Hour, t.Minute);

  body += mr.hasRise ? String(buf) : "(none)";
  if (mr.hasRise) {
    body += " @";
    body += String(mr.riseAz, 0);
    body += " deg";
  }
  body += "\nMoonset:  ";
  breakTime(mr.setTime + gmtOffset, t);
  snprintf(buf, sizeof(buf), "%02d:%02d", t.Hour, t.Minute);

  body += mr.hasRise ? String(buf) : "(none)";
  if (mr.hasSet) {
    body += " @";
    body += String(mr.setAz, 0);
    body += " deg";
  }
  body += "\nVisible now: ";
  body += mr.isVisible ? "yes" : "no";
  body += "\nMENU: refresh";
}

} // namespace

void Watchy::showMoonRise() {
  guiState = APP_STATE;
  UiTemplates::waitForAllButtonsReleased();
  uint16_t firstLine = 0;
  const int16_t lineHeight = 14;
  const int16_t margin = 10;

  UITextSpec header{};
  header.x = 52;
  header.y = 30;
  header.font = &FreeMonoBold9pt7b;
  header.fillBackground = false;
  header.invert = false;
  header.text = "Moon rise";

  UIScrollableTextSpec scroll{};
  scroll.x = margin;
  scroll.y = 50;
  scroll.w = WatchyDisplay::WIDTH - (margin * 2);
  scroll.h = WatchyDisplay::HEIGHT - 72;
  scroll.font = UiSDK::tinyMono6x8();
  scroll.fillBackground = false;
  scroll.text = "";
  scroll.textRef = nullptr;
  scroll.firstLine = firstLine;
  scroll.maxLines = 8;
  scroll.lineHeight = lineHeight;
  scroll.centered = false;
  scroll.wrapLongLines = false;
  scroll.wrapContinuationAlignRight = false;

  UIAppSpec app{};
  app.texts = &header;
  app.textCount = 1;
  app.scrollTexts = &scroll;
  app.scrollTextCount = 1;

  String body;
  UiTemplates::runRefreshableScrollableViewer(*this,
                                             app,
                                             scroll,
                                             firstLine,
                                             "BACK",
                                             "UP",
                                             "REFRESH",
                                             "DOWN",
                                             buildMoonRiseText,
                                             nullptr,
                                             body);
  showMenu(menuIndex);
}
