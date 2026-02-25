#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include <SunRise.h>

namespace {

static void buildSunRiseText(Watchy &watchy, void * /*userData*/, String &body) {
  Watchy::RTC.read(watchy.currentTime);
  time_t nowTs = makeTime(watchy.currentTime);
  double lat = watchy.settings.lat.length() ? watchy.settings.lat.toFloat() : 0.0;
  double lon = watchy.settings.lon.length() ? watchy.settings.lon.toFloat() : 0.0;

  SunRise sr;
  sr.calculate(lat, lon, nowTs);

  body.reserve(200);
  body += "Sunrise: ";
  tmElements_t t;
  breakTime(sr.riseTime + gmtOffset, t);
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", t.Hour, t.Minute);
  body += sr.hasRise ? String(buf) : "(none)";
  if (sr.hasRise) {
    body += " @";
    body += String(sr.riseAz, 0);
    body += " deg";
  }
  body += "\nSunset:  ";
  breakTime(sr.setTime + gmtOffset, t);
  snprintf(buf, sizeof(buf), "%02d:%02d", t.Hour, t.Minute);
  body += sr.hasSet ? String(buf) : "(none)";
  if (sr.hasSet) {
    body += " @";
    body += String(sr.setAz, 0);
    body += " deg";
  }
  body += "\nVisible now: ";
  body += sr.isVisible ? "yes" : "no";
  body += "\nGMT offset: ";
  body += String(gmtOffset / 3600.0f, 1);
  body += "h\nMENU: refresh";
}

} // namespace

void Watchy::showSunRise() {
  guiState = APP_STATE;
  UiTemplates::waitForAllButtonsReleased();                                            
  uint16_t firstLine = 0;
  const int16_t lineHeight = 14;
  const int16_t margin = 10;

  UITextSpec header{};
  header.x = 52;
  header.y = 36;
  header.font = &FreeMonoBold9pt7b;
  header.fillBackground = false;
  header.invert = false;
  header.text = "Sun rise";

  UIScrollableTextSpec scroll{};
  scroll.x = margin;
  scroll.y = 56;
  scroll.w = WatchyDisplay::WIDTH - (margin * 2);
  scroll.h = WatchyDisplay::HEIGHT - 78;
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
                                             buildSunRiseText,
                                             nullptr,
                                             body);
  showMenu(menuIndex);
}