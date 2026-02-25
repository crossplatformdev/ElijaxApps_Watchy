#include "../../src/watchy/Watchy.h"          //include the Watchy library
#include "soloist115pt7b.h"  //include any fonts you want to use
#include "Orbitron_Bold_16.h"
#include "profilebike.h"             //Akira bike image
#include "../../src/settings/settings.h"

class AkiraFace : public Watchy {  //inherit and extend Watchy class
public:
  AkiraFace(const watchySettings& s)
    : Watchy(s) {}
  void drawWatchFace() {  //override this method to customize how the watch face looks

    display.fillScreen(GxEPD_BLACK);
    display.drawBitmap(0, 0, profilebike, DISPLAY_WIDTH, DISPLAY_HEIGHT, GxEPD_WHITE);


    //Day of the week
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&Orbitron_Bold_16);
    display.setCursor(15, 30);
    display.println(dayStr(currentTime.Wday));

    //Day of the month
    display.setCursor(15, 55);
    display.println(monthShortStr(currentTime.Month));

    display.setCursor(45, 55);
    display.println(currentTime.Day);



    //Display Hour
    display.setFont(&soloist115pt7b);
    display.setCursor(80, 60);
    if (currentTime.Hour < 10) {  //use the currentTime struct to print latest time
      display.print("0");
    }
    display.print(currentTime.Hour);
    display.print(":");
    if (currentTime.Minute < 10) {
      display.print("0");
    }
    display.println(currentTime.Minute);


  }
};

AkiraFace m(settings);  //instantiate your watchface

void setup() {
  m.init();  //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
