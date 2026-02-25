void WatchyBrain::drawBahn(int variant, float batt) {

  const uint16_t white = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t black = UiSDK::getWatchfaceFg(BASE_POLARITY);


    // ** SETUP **

  int16_t  x1, y1, lasty;
  uint16_t w, h;
  String textstring;
  
  // ** DRAW **

  //drawbg
  display.fillScreen((variant == 0) ? white : black);
  display.fillRoundRect(2, 2, 196, 196, 8, black);
  display.fillRoundRect(6, 6, 188, 188, 5, (variant == 0) ? white : black);

  display.setFont(&DIN_1451_Engschrift_Regular64pt7b);
  display.setTextColor((variant == 0) ? black : white);
  display.setTextWrap(false);

  //draw hours
  textstring = currentTime.Hour;
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(183 - w, 100 - 5);
  display.print(textstring);

  //draw minutes
  if (currentTime.Minute < 10) {
    textstring = "0";
  } else {
    textstring = "";
  }
  textstring += currentTime.Minute;
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(183 - w, 100 + 3 + h);
  display.print(textstring);

  // draw battery
  display.fillRoundRect(16, 16, 34, 12, 4, (variant == 0) ? black : white);
  display.fillRoundRect(49, 20, 3, 4, 2, (variant == 0) ? black : white);
  display.fillRoundRect(18, 18, 30, 8, 3, (variant == 0) ? white : black);
  if (batt > 0) {
    display.fillRoundRect(20, 20, 26 * batt, 4, 2, (variant == 0) ? black : white);
  }
  
   
  lasty = 200 - 16;

  //draw steps
  textstring = sensor.getCounter();
  textstring += " steps";
  display.setFont(&DIN_1451_Engschrift_Regular12pt7b);
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.fillRoundRect(16, lasty - h - 2, w + 7, h + 4, 2, (variant == 0) ? black : white);
  display.setCursor(19, lasty - 3);
  display.setTextColor((variant == 0) ? white : black);
  display.print(textstring);
  display.setTextColor((variant == 0) ? black : white);
  lasty += -8 - h;

  // draw year
  textstring = currentTime.Year + 1970;
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(16, lasty);
  display.print(textstring);
  lasty += -20;

  // draw date
  textstring = monthShortStr(currentTime.Month);
  textstring += " ";
  textstring += currentTime.Day;
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(16, lasty);
  display.print(textstring);
  lasty += -20;

  // draw day
  textstring = dayStr(currentTime.Wday);
  display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(16, lasty);
  display.print(textstring);
  lasty += -40;

}
