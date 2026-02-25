#include "../../src/watchy/Watchy.h" //include the Watchy library

#include "Fonts/Roboto_Medium16pt7b.h" //include any fonts you want to use

#include "Fonts/Roboto_Medium7pt7b.h" //include any fonts you want to use


//RTC_DATA_ATTR bool BLE_CONFIGURED =  false;
RTC_DATA_ATTR int stored_oldsteps[7] ;


class WatchFace: public Watchy { //inherit and extend Watchy class
  public:

    void drawRightAligned(String text, byte x, byte y) {
      int16_t x1, y1;
      uint16_t w1, h1;
      display.getTextBounds(text, 0, 0, & x1, & y1, & w1, & h1);
      display.setCursor(x - w1, y);
      display.print(text);
    }

  void drawWrapCentred(String text, byte centerx, byte centery, byte bx, byte yspacing) {
    // Only one space - no verylongwordthatdoesnotrwap - no carriage returns
    const byte maxlines = 20;
    byte line = 0;
    int startnewlinepos[maxlines + 1];
    int stringpos = 0;
    int nextspace = 0;
    int lastspace = 0;
    String linetext;

    int16_t x1, y1, cx, cy;
    uint16_t w1, h1;

    display.setTextWrap(false); // Needs to test if long - so we do not want auto wrapping

    startnewlinepos[line] = 0;
    while (stringpos < text.length()) {
      nextspace = text.indexOf(' ', stringpos);
      if (nextspace == -1) nextspace = text.length();
      // Careful will wrap on letter so say 171 will be max if next char is
      display.getTextBounds(text.substring(startnewlinepos[line], nextspace), 0, 0, & x1, & y1, & w1, & h1);
      if (w1 > bx) {
        // wider at next space
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
      display.getTextBounds(linetext, 0, 0, & x1, & y1, & w1, & h1);
      cx = centerx - w1 / 2;
      cy = centery - (line * (yspacing / 2)) + (i * yspacing) + (yspacing / 3);
      display.setCursor(cx, cy);
      display.print(linetext);
    }
  }

  String stepsink(String steps, byte leftdigits) {
    // Truncated not rounded
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

  // Used https://github.com/peerdavid/Watchy/blob/master/examples/WatchFaces/David/Watchy_Base.cpp
  uint8_t getBattery() {
    float voltage = getBatteryVoltage() + 0.25;

    uint8_t percentage = 2808.3808 * pow(voltage, 4) -
      43560.9157 * pow(voltage, 3) +
      252848.5888 * pow(voltage, 2) -
      650767.4615 * voltage +
      626532.5703;
    percentage = min((uint8_t) 100, percentage);
    percentage = max((uint8_t) 0, percentage);
    return percentage;
  }

  void drawWatchFace() { //override this method to customize how the watch face looks
    // Using Logic in https://github.com/bmsleight/wasp-os/blob/master/wasp/apps/word_clock.py
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
    byte minute_round = currentTime.Minute % 5;
    String steps = "";
    String datetext = "";
    String batterytext = "";

    if (currentTime.Minute > 32) {
      hour_word = hours_a[(currentTime.Hour + 1) % 24];
    } else {
      hour_word = hours_a[currentTime.Hour % 24];
    }

    if (hour_word != "midnight"
      and hour_word != "twelve") {
      if (currentTime.Hour >= 22) {
        part_day = "at night";
      } else if (currentTime.Hour >= 18) {
        part_day = "in the evening";
      } else if (currentTime.Hour >= 12) {
        part_day = "in the afternoon";
      } else if (currentTime.Hour >= 6) {
        part_day = "in the morning";
      } else if (currentTime.Hour >= 3) {
        part_day = "in the early hours";
      } else {
        part_day = "at night";
      }
    }

    if (minute_round == 4) minute_words = "almost";
    if (minute_round == 3) minute_words = "coming up to";
    if (minute_round == 2) minute_words = "a little after";
    if (minute_round == 1) minute_words = "just gone";

    // Yes I coudl convert to array loook-up.. but readable....
    if (currentTime.Minute > 57) {
      hour_part = "";
    } else if (currentTime.Minute > 52) {
      hour_part = "five to";
    } else if (currentTime.Minute > 47) {
      hour_part = "ten to";
    } else if (currentTime.Minute > 42) {
      hour_part = "quarter to";
    } else if (currentTime.Minute > 37) {
      hour_part = "twenty to";
    } else if (currentTime.Minute > 32) {
      hour_part = "twenty-five to";
    } else if (currentTime.Minute > 27) {
      hour_part = "half past";
    } else if (currentTime.Minute > 22) {
      hour_part = "twenty-five past";
    } else if (currentTime.Minute > 17) {
      hour_part = "twenty past";
    } else if (currentTime.Minute > 12) {
      hour_part = "quarter past";
    } else if (currentTime.Minute > 7) {
      hour_part = "ten past";
    } else if (currentTime.Minute > 2) {
      hour_part = "five past";
    }

    if (minute_words != "") whole_phase = minute_words + " ";
    if (hour_part != "") whole_phase += hour_part + " ";
    if (hour_word != "") whole_phase += hour_word + " ";
    if (part_day != "") whole_phase += part_day + " ";

    capital = whole_phase.substring(0, 1);
    capital.toUpperCase();
    whole_phase = capital + whole_phase.substring(1);
    // Longest Test phase
    // whole_phase ="Coming up to twenty-five past eleven in the afternoon";
    // whole_phase ="Coming up to twenty-five past four in the afternoon";
    // screenshot 1
//    whole_phase = "Midnight";    
//    whole_phase = "Coming up to twenty-five past five in afternoon";
//    whole_phase = "Almost twenty-five past five in afternoon";
//    whole_phase = "Twenty-five past five in afternoon";
//    whole_phase = "Just gone twenty-five past five in afternoon";
//    whole_phase = "A little after twenty-five past five in afternoon";
//    whole_phase = "Coming up to half past five in afternoon";
    

    // Clear background
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(true);

    display.setFont( & Roboto_Medium16pt7b);
    drawWrapCentred(whole_phase, 100, 100 - 12, 200, 32);

    // Set step to zero at midnight, but store 7 days
    if (currentTime.Hour == 00 && currentTime.Minute == 00) {
      for (int i = 6; i >= 1; i--) {
        stored_oldsteps[i] = stored_oldsteps[i - 1];
      }
      sensor.resetStepCounter();
    }

    //Load steps in k format ie. 12,345 is 12.3
    stored_oldsteps[0] = sensor.getCounter();
    steps = "Steps:" + stepsink(String(stored_oldsteps[0]), 1) + " " + stepsink(String(stored_oldsteps[1]), 1) + " ";
    for (int i = 2; i <= 6; i++) {
      steps += stepsink(String(stored_oldsteps[i]), 0) + " ";
    }

    // Sample steps used for screenshots
    // steps = "Steps:14.2 13.2 12 11 10 19 18";
    display.setFont( & Roboto_Medium7pt7b);
    display.setCursor(0, 196);
    display.print(steps);

    // Draw Battery
    //batterytext = "Battery:" + String(getBattery()) + "%";
    batterytext = "B: " + String(getBattery()) + "%";
    drawRightAligned(batterytext, 200, 180);

    // Draw date
    datetext = dayStr(currentTime.Wday);
    datetext += ", " + String(currentTime.Day) + "/" + monthShortStr(currentTime.Month) + "/" + String(currentTime.Year + 1970 - 2000);
    display.setCursor(0, 180);
    display.print(datetext);

  }
};

WatchFace m; //instantiate your watchface


void setup() {
//  Serial.begin(115200); 
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
