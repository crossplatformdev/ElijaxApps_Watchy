#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

namespace {
struct SetTimeState {
  Watchy *watchy = nullptr;
  volatile bool exitRequested = false;
  bool dirty = true;

  uint8_t minute = 0;
  uint8_t hour = 0;
  uint8_t day = 1;
  uint8_t month = 1;
  uint16_t yearCal = 1970; // calendar year

  int16_t tzOffsetMin = 0;
  int8_t setIndex = SET_HOUR;
  int8_t blink = 0;
  uint32_t lastBlinkMs = 0;
};

static SetTimeState sSetTime;

static void setTimeBack(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  if (sSetTime.setIndex != SET_HOUR) {
    sSetTime.setIndex--;
    sSetTime.dirty = true;
  }
}

static void setTimeMenu(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);  
  sSetTime.setIndex++;
  if (sSetTime.setIndex > SET_TZ) {
    sSetTime.exitRequested = true;
    return;
  }
  sSetTime.dirty = true;
}

static void setTimeDown(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);
  sSetTime.blink = 1;
  switch (sSetTime.setIndex) {
    case SET_HOUR:
      sSetTime.hour = (sSetTime.hour == 23) ? 0 : static_cast<uint8_t>(sSetTime.hour - 1);
      break;
    case SET_MINUTE:
      sSetTime.minute = (sSetTime.minute == 59) ? 0 : static_cast<uint8_t>(sSetTime.minute - 1);
      break;
    case SET_YEAR:
      sSetTime.yearCal = (sSetTime.yearCal >= 2099) ? 1970 : static_cast<uint16_t>(sSetTime.yearCal - 1);
      break;
    case SET_MONTH:
      sSetTime.month = (sSetTime.month == 12) ? 1 : static_cast<uint8_t>(sSetTime.month - 1);
      break;
    case SET_DAY:
      sSetTime.day = (sSetTime.day == 31) ? 1 : static_cast<uint8_t>(sSetTime.day - 1);
      break;
    case SET_TZ:
      if (sSetTime.tzOffsetMin < 840) {
        sSetTime.tzOffsetMin = static_cast<int16_t>(sSetTime.tzOffsetMin - 30);
      }
      break;
    default:
      break;
  }
  sSetTime.dirty = true;
}

static void setTimeUp(Watchy *watchy) {
  UiTemplates::waitForAllButtonsReleased(50, 100);

  sSetTime.blink = 1;
  switch (sSetTime.setIndex) {
    case SET_HOUR:
      sSetTime.hour = (sSetTime.hour == 0) ? 23 : static_cast<uint8_t>(sSetTime.hour + 1);
      break;
    case SET_MINUTE:
      sSetTime.minute = (sSetTime.minute == 0) ? 59 : static_cast<uint8_t>(sSetTime.minute + 1);
      break;
    case SET_YEAR:
      sSetTime.yearCal = (sSetTime.yearCal <= 1970) ? 2099 : static_cast<uint16_t>(sSetTime.yearCal + 1);
      break;
    case SET_MONTH:
      sSetTime.month = (sSetTime.month == 1) ? 12 : static_cast<uint8_t>(sSetTime.month + 1);
      break;
    case SET_DAY:
      sSetTime.day = (sSetTime.day == 1) ? 31 : static_cast<uint8_t>(sSetTime.day - 1);
      break;
    case SET_TZ:
      if (sSetTime.tzOffsetMin > -720) {
        sSetTime.tzOffsetMin = static_cast<int16_t>(sSetTime.tzOffsetMin + 30);
      }
      break;
    default:
      break;
  }
  sSetTime.dirty = true;
}
} // namespace

void Watchy::showSetTime() {

  guiState = APP_STATE;

  RTC.read(currentTime);

  sSetTime.watchy = this;
  sSetTime.exitRequested = false;
  sSetTime.dirty = true;
  sSetTime.minute = currentTime.Minute;
  sSetTime.hour = currentTime.Hour;
  sSetTime.day = currentTime.Day;
  sSetTime.month = currentTime.Month;
  sSetTime.yearCal = tmYearToCalendar(currentTime.Year);
  sSetTime.tzOffsetMin = static_cast<int16_t>(gmtOffset / 60);
  sSetTime.setIndex = SET_HOUR;
  sSetTime.blink = 0;
  sSetTime.lastBlinkMs = millis();

  UiTemplates::waitForAllButtonsReleased();

  setButtonHandlers(setTimeBack, setTimeUp, setTimeMenu, setTimeDown);

  UIControlsRowLayout controls[4] = {
      {"BACK", &Watchy::backPressed},
      {"UP", &Watchy::upPressed},
      {"NEXT", &Watchy::menuPressed},
      {"DOWN", &Watchy::downPressed},
  };

  while (!sSetTime.exitRequested) {
    UiSDK::renderControlsRow(*this, controls);

    const uint32_t nowMs = millis();
    if (nowMs - sSetTime.lastBlinkMs >= 500) {
      sSetTime.lastBlinkMs = nowMs;
      sSetTime.blink = 1 - sSetTime.blink;
      sSetTime.dirty = true;
    }

    if (!sSetTime.dirty) {
      delay(10);
      continue;
    }
    sSetTime.dirty = false;

    UiSDK::initScreen(display);
    // Build formatted strings
    String hourStr   = (sSetTime.hour   < 10 ? "0" : "") + String(sSetTime.hour);
    String minuteStr = (sSetTime.minute < 10 ? "0" : "") + String(sSetTime.minute);
    String yearStr   = String(sSetTime.yearCal);
    String monthStr  = (sSetTime.month  < 10 ? "0" : "") + String(sSetTime.month);
    String dayStr    = (sSetTime.day    < 10 ? "0" : "") + String(sSetTime.day);

    // Format timezone string like GMT+05:30 or GMT-03:00
    int16_t tzAbs    = abs(sSetTime.tzOffsetMin);
    int8_t tzHours   = tzAbs / 60;
    int8_t tzMinutes = tzAbs % 60;
    char tzBuf[12];
    snprintf(tzBuf, sizeof(tzBuf), "GMT%c%02d:%02d", (sSetTime.tzOffsetMin >= 0 ? '+' : '-'), tzHours, tzMinutes);

      // Compute layout with explicit vertical spacing to avoid overlap
      int16_t x1, y1;
      uint16_t w, h;

      const int16_t timeY = 80;
      const int16_t hourX = 5;
      display.setFont(&DSEG7_Classic_Bold_53);
      display.getTextBounds(hourStr, hourX, timeY, &x1, &y1, &w, &h);
      const uint16_t timeLineHeight = h;
      int16_t colonX  = 100; // original colon cursor
      const int16_t minuteX = 108; // original minute cursor

      const int16_t dateY = timeY + (int16_t)timeLineHeight + 24; // leave room under time
      const int16_t yearX = 45;
      display.setFont(&FreeMonoBold9pt7b);
      display.getTextBounds(yearStr, yearX, dateY, &x1, &y1, &w, &h);
      const uint16_t dateLineHeight = h;
      int16_t slash1X = yearX + (int16_t)w;
      display.getTextBounds("/", slash1X, dateY, &x1, &y1, &w, &h);
      int16_t monthX  = slash1X + (int16_t)w;
      display.getTextBounds(monthStr, monthX, dateY, &x1, &y1, &w, &h);
      int16_t slash2X = monthX + (int16_t)w;
      display.getTextBounds("/", slash2X, dateY, &x1, &y1, &w, &h);
      int16_t dayX    = slash2X + (int16_t)w;

      const int16_t tzY   = dateY + (int16_t)dateLineHeight + 22;
      const int16_t tzX   = 52;

    // Prepare text specs for each segment so we can blink them individually
    UITextSpec texts[9];

    // Hour
    texts[0].x               = hourX;
    texts[0].y               = timeY;
    texts[0].w               = 0;
    texts[0].h               = 0;
    texts[0].font            = &DSEG7_Classic_Bold_53;
    texts[0].fillBackground  = false;
    texts[0].text            = hourStr;
    texts[0].invert          = false;

    // Colon
    texts[1].x               = colonX;
    texts[1].y               = timeY;
    texts[1].w               = 0;
    texts[1].h               = 0;
    texts[1].font            = &DSEG7_Classic_Bold_53;
    texts[1].fillBackground  = false;
    texts[1].text            = String(":");
    texts[1].invert          = false;

    // Minute
    texts[2].x               = minuteX;
    texts[2].y               = timeY;
    texts[2].w               = 0;
    texts[2].h               = 0;
    texts[2].font            = &DSEG7_Classic_Bold_53;
    texts[2].fillBackground  = false;
    texts[2].text            = minuteStr;
    texts[2].invert          = false;

    // Year
    texts[3].x               = yearX;
    texts[3].y               = dateY;
    texts[3].w               = 0;
    texts[3].h               = 0;
    texts[3].font            = &FreeMonoBold9pt7b;
    texts[3].fillBackground  = false;
    texts[3].text            = yearStr;
    texts[3].invert          = false;

    // First slash
    texts[4].x               = slash1X;
    texts[4].y               = dateY;
    texts[4].w               = 0;
    texts[4].h               = 0;
    texts[4].font            = &FreeMonoBold9pt7b;
    texts[4].fillBackground  = false;
    texts[4].text            = String("/");
    texts[4].invert          = false;

    // Month
    texts[5].x               = monthX;
    texts[5].y               = dateY;
    texts[5].w               = 0;
    texts[5].h               = 0;
    texts[5].font            = &FreeMonoBold9pt7b;
    texts[5].fillBackground  = false;
    texts[5].text            = monthStr;
    texts[5].invert          = false;

    // Second slash
    texts[6].x               = slash2X;
    texts[6].y               = dateY;
    texts[6].w               = 0;
    texts[6].h               = 0;
    texts[6].font            = &FreeMonoBold9pt7b;
    texts[6].fillBackground  = false;
    texts[6].text            = String("/");
    texts[6].invert          = false;

    // Day
    texts[7].x               = dayX;
    texts[7].y               = dateY;
    texts[7].w               = 0;
    texts[7].h               = 0;
    texts[7].font            = &FreeMonoBold9pt7b;
    texts[7].fillBackground  = false;
    texts[7].text            = dayStr;
    texts[7].invert          = false;

    // Timezone
    texts[8].x               = tzX;
    texts[8].y               = tzY;
    texts[8].w               = 0;
    texts[8].h               = 0;
    texts[8].font            = &FreeMonoBold9pt7b;
    texts[8].fillBackground  = false;
    texts[8].text            = String(tzBuf);
    texts[8].invert          = false;

    // Blink/highlight the active field while keeping layout stable
    auto fillActiveWithBlink = [&](UITextSpec &spec) {
        spec.invert = (sSetTime.blink == 1);
    };

    switch (sSetTime.setIndex) {
      case SET_HOUR:   fillActiveWithBlink(texts[0]); break;
      case SET_MINUTE: fillActiveWithBlink(texts[2]); break;
      case SET_YEAR:   fillActiveWithBlink(texts[3]); break;
      case SET_MONTH:  fillActiveWithBlink(texts[5]); break;
      case SET_DAY:    fillActiveWithBlink(texts[7]); break;
      case SET_TZ:     fillActiveWithBlink(texts[8]); break;
      default: break;
    }

    UIAppSpec app{};
    app.texts        = texts;
    app.textCount    = 9;
    app.images       = nullptr;
    app.imageCount   = 0;
    app.menus        = nullptr;
    app.menuCount    = 0;
    app.buttons      = nullptr;
    app.buttonCount  = 0;
    app.scrollTexts  = nullptr;
    app.scrollTextCount = 0;
    app.callbacks    = nullptr;
    app.callbackCount = 0;

    app.controls[0] = controls[0];
    app.controls[1] = controls[1];
    app.controls[2] = controls[2];
    app.controls[3] = controls[3];

    UiSDK::renderApp(*this, app);

    gmtOffset = static_cast<long>(sSetTime.tzOffsetMin) * 60L;
    settings.gmtOffset = static_cast<int>(gmtOffset);
  }

  clearButtonHandlers();

  tmElements_t tm;
  tm.Month  = sSetTime.month;
  tm.Day    = sSetTime.day;
  tm.Year   = static_cast<uint8_t>(sSetTime.yearCal - 1970);
  tm.Hour   = sSetTime.hour;
  tm.Minute = sSetTime.minute;
  tm.Second = 0;

  RTC.set(tm);
  currentTime = tm;
  
  // Persist timezone (seconds)
  gmtOffset        = static_cast<long>(sSetTime.tzOffsetMin) * 60L;
  settings.gmtOffset = static_cast<int>(gmtOffset);
  delay(100); // allow blink/highlight to be perceivable on e-paper
  showMenu(menuIndex);
}
