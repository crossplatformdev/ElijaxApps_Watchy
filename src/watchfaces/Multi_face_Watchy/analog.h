void WatchyBrain::drawClockface(float batt) {

          UiSDK::initScreen(display);
          const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
          const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

         //background
          display.drawBitmap(0, 0, clockFace_square2, 200, 200, fg);
          display.fillCircle(100,100,3,fg);
   
          //Logo
          display.setCursor(80, 40);
          display.setFont(&FreeSerifBoldItalic9pt7b);
          display.setTextColor(fg);
          display.print("Gabel ");
          display.setCursor(54, 60);
          display.setFont(&FreeSerifItalic9pt7b);
          display.print("Chronometrie");

          //date
          String dateDay = "";
          if(currentTime.Day < 10){
            dateDay += "0";
          }
          dateDay += currentTime.Day;
          display.fillRect(140, 88, 27, 24, fg);
          display.setFont(&FreeSerifBold12pt7b);
          display.setTextColor(bg);
          display.setCursor(141, 107);
          display.print(dateDay);

          //weekday
          String wDay = dayShortStr(currentTime.Wday);
          wDay = wDay.substring(0,wDay.length() - 1);
          display.fillRect(82, 140, 33, 22, fg);
          display.setFont(&FreeSerifBold12pt7b);
          display.setTextColor(bg);
          display.setCursor(81, 158);
          display.print(wDay);

          // draw battery
          display.drawCircleHelper(45, 100, 20, 2, fg);
          display.drawCircleHelper(45, 100, 20, 4, fg);
          display.drawPixel(65, 100, fg);
          display.drawFastVLine(45, 79, 4, fg);
          display.drawFastHLine(63, 100, 4, fg);
          display.drawFastVLine(45, 118, 4, fg);
          display.setFont(&FreeSerifBoldItalic4pt7b);
          display.setTextColor(fg);
          display.setCursor(44, 76);
          display.print("1");
          display.setCursor(70, 97);
          display.print("1");
          display.setCursor(69, 101);
          display.print("--");
          display.drawPixel(71, 100, fg);
          display.setCursor(69, 107);
          display.print("2");
          display.setCursor(43, 127);
          display.print("0");
          display.fillCircle(45, 100, 2, fg);
          double batteryCurrent = (getBatteryVoltage() - 3.3) / 0.9;
          double batteryAngle = batteryCurrent * 180;
          double radBattery = ((batteryAngle) * 71) / 4068.0;
          double bx1 = 45 + (sin(radBattery) * 16);
          double by1 = 100 + (cos(radBattery) * 16);
          display.drawLine(45, 100, (int)bx1, (int)by1, fg);

          //minute pointer
          int currentMinute = currentTime.Minute;
          int minuteAngle = currentMinute * 6;
          double radMinute = ((minuteAngle + 180) * 71) / 4068.0;
          double mx1 = 100 - (sin(radMinute) * 85);
          double my1 = 100 + (cos(radMinute) * 85);
          display.drawLine(97, 103, (int)mx1, (int)my1, fg);
          display.drawLine(98, 102, (int)mx1, (int)my1, fg);
          display.drawLine(99, 101, (int)mx1, (int)my1, fg);
          display.drawLine(97, 97, (int)mx1, (int)my1, fg);
          display.drawLine(98, 98, (int)mx1, (int)my1, fg);
          display.drawLine(99, 99, (int)mx1, (int)my1, fg);
          display.drawLine(100, 100, (int)mx1, (int)my1, fg);
          display.drawLine(101, 101, (int)mx1, (int)my1, fg);
          display.drawLine(102, 102, (int)mx1, (int)my1, fg);
          display.drawLine(103, 103, (int)mx1, (int)my1, fg);
          display.drawLine(101, 99, (int)mx1, (int)my1, fg);
          display.drawLine(102, 98, (int)mx1, (int)my1, fg);
          display.drawLine(103, 97, (int)mx1, (int)my1, fg);


          //hour pointer
          //To change Daylight Savings Time, see the settings.h section
          int currentHour= currentTime.Hour;
          double hourAngle = (currentHour * 30) + currentMinute * 0.5;
          double radHour = ((hourAngle + 180) * 71) / 4068.0;
          double hx1 = 100 - (sin(radHour) * 55);
          double hy1 = 100 + (cos(radHour) * 55);
          display.drawLine(96, 104, (int)hx1, (int)hy1, bg);
          display.drawLine(96, 96, (int)hx1, (int)hy1, bg);
          display.drawLine(104, 104, (int)hx1, (int)hy1, bg);
          display.drawLine(104, 96, (int)hx1, (int)hy1, bg);
          display.drawLine(97, 103, (int)hx1, (int)hy1, fg);
          display.drawLine(98, 102, (int)hx1, (int)hy1, fg);
          display.drawLine(99, 101, (int)hx1, (int)hy1, fg);
          display.drawLine(97, 97, (int)hx1, (int)hy1, fg);
          display.drawLine(98, 98, (int)hx1, (int)hy1, fg);
          display.drawLine(99, 99, (int)hx1, (int)hy1, fg);
          display.drawLine(101, 101, (int)hx1, (int)hy1, fg);
          display.drawLine(102, 102, (int)hx1, (int)hy1, fg);
          display.drawLine(103, 103, (int)hx1, (int)hy1, fg);
          display.drawLine(101, 99, (int)hx1, (int)hy1, fg);
          display.drawLine(102, 98, (int)hx1, (int)hy1, fg);
          display.drawLine(103, 97, (int)hx1, (int)hy1, fg);


        }
/*};

WatchFace m(settings); //instantiate your watchface, added (settings) wre

void setup() {
  m.init(); //call init in setup
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
*/
