#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"
#include <MoonPhase.h>

namespace {

struct MoonPhaseCtx {
  MoonPhase mp;
};

static void buildMoonPhaseText(Watchy &watchy, void *userData, String &body) {
  MoonPhaseCtx *ctx = static_cast<MoonPhaseCtx *>(userData);
  if (ctx == nullptr) {
    return;
  }

  Watchy::RTC.read(watchy.currentTime);
  time_t nowTs = makeTime(watchy.currentTime);
  ctx->mp.calculate(nowTs);

  body.reserve(256);
  body += "Phase: ";
  body += ctx->mp.phaseName ? ctx->mp.phaseName : "-";
  body += "\nZodiac: ";
  body += ctx->mp.zodiacName ? ctx->mp.zodiacName : "-";
  body += "\nIllumination: ";
  body += String(ctx->mp.fraction * 100.0f, 1);
  body += "%\nAge: ";
  body += String(ctx->mp.age, 1);
  body += " days\nDistance: ";
  body += String(ctx->mp.distance, 2);
  body += " Earth R\nPhase frac: ";
  body += String(ctx->mp.phase, 3);
  body += "\nLat/Lon: ";
  body += String(ctx->mp.latitude, 2);
  body += ", ";
  body += String(ctx->mp.longitude, 2);
  body += " deg\nMENU: refresh";
}

} // namespace

void Watchy::showMoonPhase() {
  guiState = APP_STATE;
  UiTemplates::waitForAllButtonsReleased();

  uint16_t firstLine = 0;
  const int16_t lineHeight = 14;
  const int16_t margin = 10;

  UITextSpec header{};
  header.x = 48;
  header.y = 36;
  header.w = 0;
  header.h = 0;
  header.font = &FreeMonoBold9pt7b;
  header.fillBackground = false;
  header.invert = false;
  header.text = "Moon phase";

  UIScrollableTextSpec scroll{};
  scroll.x = margin;
  scroll.y = 56;
  scroll.w = WatchyDisplay::WIDTH - (margin * 2);
  scroll.h = WatchyDisplay::HEIGHT - 78;
  scroll.font = UiSDK::tinyMono6x8();
  scroll.fillBackground = false;
  scroll.centered = false;
  scroll.text = "";
  scroll.textRef = nullptr;
  scroll.firstLine = firstLine;
  scroll.maxLines = 9;
  scroll.lineHeight = lineHeight;
  scroll.wrapLongLines = false;
  scroll.wrapContinuationAlignRight = false;

  UIAppSpec app{};
  app.texts = &header;
  app.textCount = 1;
  app.scrollTexts = &scroll;
  app.scrollTextCount = 1;

  MoonPhaseCtx ctx;
  String body;
  UiTemplates::runRefreshableScrollableViewer(*this,
                                             app,
                                             scroll,
                                             firstLine,
                                             "BACK",
                                             "UP",
                                             "REFRESH",
                                             "DOWN",
                                             buildMoonPhaseText,
                                             &ctx,
                                             body);
  showMenu(menuIndex);
}
