#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "Fonts/Roboto_Medium16pt7b.h"
#include "Fonts/Roboto_Medium7pt7b.h"

RTC_DATA_ATTR static int stored_oldsteps[7];

namespace {

void drawRightAligned(Watchy &watchy, String text, byte x, byte y) {
  int16_t x1, y1;
  uint16_t w1, h1;
  UiSDK::getTextBounds(watchy.display, text, 0, 0, &x1, &y1, &w1, &h1);
  UiSDK::setCursor(watchy.display, x - w1, y);
  UiSDK::print(watchy.display, text);
}

void drawWrapCentred(Watchy &watchy, String text, byte centerx, byte centery, byte bx, byte yspacing) {
  const byte maxlines = 20;
  byte line = 0;
  int startnewlinepos[maxlines + 1];
  int stringpos = 0;
  int nextspace = 0;
  int lastspace = 0;
  String linetext;

  int16_t x1, y1, cx, cy;
  uint16_t w1, h1;

  UiSDK::setTextWrap(watchy.display, false);

  startnewlinepos[line] = 0;
  while (stringpos < text.length()) {
    nextspace = text.indexOf(' ', stringpos);
    if (nextspace == -1) nextspace = text.length();
    UiSDK::getTextBounds(watchy.display, text.substring(startnewlinepos[line], nextspace), 0, 0, &x1, &y1, &w1, &h1);
    if (w1 > bx) {
      line++;
      startnewlinepos[line] = lastspace;
      stringpos = lastspace + 1;
    } else {
      lastspace = nextspace;
      stringpos = nextspace + 1;
    }
  }
  startnewlinepos[line + 1] = text.length();

  for (int i = 0; i <= line; i++) {
    linetext = text.substring(startnewlinepos[i], startnewlinepos[i + 1]);
    UiSDK::getTextBounds(watchy.display, linetext, 0, 0, &x1, &y1, &w1, &h1);
    cx = centerx - w1 / 2;
    cy = centery - (line * (yspacing / 2)) + (i * yspacing) + (yspacing / 3);
    UiSDK::setCursor(watchy.display, cx, cy);
    UiSDK::print(watchy.display, linetext);
  }
}

String stepsink(String steps, byte leftdigits) {
  String stepsink = "";
  if (steps.length() > 3) {
    stepsink = steps.substring(0, steps.length() - 3);
  } else {
    stepsink = "0";
    if (leftdigits > 0) {
      while(steps.length() < 3) {
        steps = "0" + steps;
      }
    }
  }
  if (leftdigits > 0) {
    stepsink += ".";
    stepsink += steps.substring(steps.length() - 3, steps.length() - 3 + leftdigits);
  }
  return (stepsink);
}

uint8_t getBattery(Watchy &watchy) {
  float voltage = watchy.getBatteryVoltage() + 0.25;

  uint8_t percentage = 2808.3808 * pow(voltage, 4) -
    43560.9157 * pow(voltage, 3) +
    252848.5888 * pow(voltage, 2) -
    650767.4615 * voltage +
    626532.5703;
  percentage = min((uint8_t) 100, percentage);
  percentage = max((uint8_t) 0, percentage);
  return percentage;
}

} // namespace

void showWatchFace_Exactly_Words(Watchy &watchy) {
  const char * hours_a[24] = {
    "midnight",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven",
    "twelve",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "eleven"
  };
  String part_day = "";
  String hour_word = "";
  String hour_part = "";
  String minute_words = "";
  String whole_phase = "";
  String capital = "";
  byte current_line = 0;
  byte minute_round = watchy.currentTime.Minute % 5;
  String steps = "";
  String datetext = "";
  String batterytext = "";

  if (watchy.currentTime.Minute > 32) {
    hour_word = hours_a[(watchy.currentTime.Hour + 1) % 24];
  } else {
    hour_word = hours_a[watchy.currentTime.Hour % 24];
  }

  if (hour_word != "midnight"
    and hour_word != "twelve") {
    if (watchy.currentTime.Hour >= 22) {
      part_day = "at night";
    } else if (watchy.currentTime.Hour >= 18) {
      part_day = "in the evening";
    } else if (watchy.currentTime.Hour >= 12) {
      part_day = "in the afternoon";
    } else if (watchy.currentTime.Hour >= 6) {
      part_day = "in the morning";
    } else if (watchy.currentTime.Hour >= 3) {
      part_day = "in the early hours";
    } else {
      part_day = "at night";
    }
  }

  if (minute_round == 4) minute_words = "almost";
  if (minute_round == 3) minute_words = "coming up to";
  if (minute_round == 2) minute_words = "a little after";
  if (minute_round == 1) minute_words = "just gone";

  if (watchy.currentTime.Minute > 57) {
    hour_part = "";
  } else if (watchy.currentTime.Minute > 52) {
    hour_part = "five to";
  } else if (watchy.currentTime.Minute > 47) {
    hour_part = "ten to";
  } else if (watchy.currentTime.Minute > 42) {
    hour_part = "quarter to";
  } else if (watchy.currentTime.Minute > 37) {
    hour_part = "twenty to";
  } else if (watchy.currentTime.Minute > 32) {
    hour_part = "twenty-five to";
  } else if (watchy.currentTime.Minute > 27) {
    hour_part = "half past";
  } else if (watchy.currentTime.Minute > 22) {
    hour_part = "twenty-five past";
  } else if (watchy.currentTime.Minute > 17) {
    hour_part = "twenty past";
  } else if (watchy.currentTime.Minute > 12) {
    hour_part = "quarter past";
  } else if (watchy.currentTime.Minute > 7) {
    hour_part = "ten past";
  } else if (watchy.currentTime.Minute > 2) {
    hour_part = "five past";
  }

  if (minute_words != "") whole_phase = minute_words + " ";
  if (hour_part != "") whole_phase += hour_part + " ";
  if (hour_word != "") whole_phase += hour_word + " ";
  if (part_day != "") whole_phase += part_day + " ";

  capital = whole_phase.substring(0, 1);
  capital.toUpperCase();
  whole_phase = capital + whole_phase.substring(1);

  UiSDK::initScreen(watchy.display);
  UiSDK::setTextColor(watchy.display, UiSDK::getWatchfaceFg(BASE_POLARITY));
  UiSDK::setTextWrap(watchy.display, true);

  UiSDK::setFont(watchy.display, &Roboto_Medium16pt7b);
  drawWrapCentred(watchy, whole_phase, 100, 100 - 12, 200, 32);

  if (watchy.currentTime.Hour == 00 && watchy.currentTime.Minute == 00) {
    for (int i = 6; i >= 1; i--) {
      stored_oldsteps[i] = stored_oldsteps[i - 1];
    }
  }

  stored_oldsteps[0] = sensor.getCounter();
  steps = "Steps:" + stepsink(String(stored_oldsteps[0]), 1) + " " + stepsink(String(stored_oldsteps[1]), 1) + " ";
  for (int i = 2; i <= 6; i++) {
    steps += stepsink(String(stored_oldsteps[i]), 0) + " ";
  }

  UiSDK::setFont(watchy.display, &Roboto_Medium7pt7b);
  UiSDK::setCursor(watchy.display, 0, 196);
  UiSDK::print(watchy.display, steps);

  batterytext = "B: " + String(getBattery(watchy)) + "%";
  drawRightAligned(watchy, batterytext, 200, 180);

  datetext = dayStr(watchy.currentTime.Wday);
  datetext += ", " + String(watchy.currentTime.Day) + "/" + monthShortStr(watchy.currentTime.Month) + "/" + String(watchy.currentTime.Year + 1970 - 2000);
  UiSDK::setCursor(watchy.display, 0, 180);
  UiSDK::print(watchy.display, datetext);
}
