#include "HobbitTime.h"

#include "../../sdk/UiSDK.h"

void HobbitTime::drawCentered(String text, int y2) {
  int16_t  x1, y1;
  uint16_t w1, h1;
  
  UiSDK::getTextBounds(display, text, 0, 0, &x1, &y1, &w1, &h1);
  UiSDK::setCursor(display, 100-w1/2,y2+h1/2);
  UiSDK::print(display, text);
}
  
void HobbitTime::drawWatchFace() { //override this method to customize how the watch face looks
  String textstring = "";
  const char *lines [25][3] = {
    {"Midnight","snack"},
    {"Sleep"},
    {"Sleep"},
    {"Sleep"},
    {"Sleep"},
    {"Sleep"},
    {"Almost","breakfast"},
    {"Breakfast"},
    {"Almost","second","breakfast"},
    {"Second","breakfast"},
    {"Almost","elevenses"},
    {"Elevenses"},
    {"Luncheon"},
    {"After","lunch","nap"},
    {"Afternoon","tea"},
    {"Three-ish"},
    {"Almost","dinner"},
    {"Dinner"},
    {"Almost","supper"},
    {"Supper"},
    {"Eight-ish"},
    {"Nine-ish"},
    {"Sleep"},
    {"Sleep"},
    {"Midnight","snack"}
  };
  
  const char linecount[25]= {
    2,
    1,
    1,
    1,
    1,
    1,
    2,
    1,
    3,
    2,
    2,
    1,
    1,
    3,
    2,
    1,
    2,
    1,
    2,
    1,
    1,
    1,
    1,
    1,
    2
  };

  UiSDK::initScreen(display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::setTextColor(display, fgColor);

  //drawtime
  UiSDK::setFont(display, &NunitoSans_Black12pt7b);
  textstring = currentTime.Hour;
  textstring += ":";
  if (currentTime.Minute<10){
    textstring += "0";
  }
  textstring += currentTime.Minute;
  drawCentered(textstring,15);

  //drawlabel
  UiSDK::setFont(display, &NunitoSans_Black18pt7b);
  if (linecount[currentTime.Hour] == 1) {
    drawCentered(lines[currentTime.Hour][0],100);
  } else if (linecount[currentTime.Hour] == 2){
    drawCentered(lines[currentTime.Hour][0],86);
    drawCentered(lines[currentTime.Hour][1],114);
  } else if (linecount[currentTime.Hour] == 3) {
    drawCentered(lines[currentTime.Hour][0],72);
    drawCentered(lines[currentTime.Hour][1],100);
    drawCentered(lines[currentTime.Hour][2],128);
  }
}

void showWatchFace_Hobbit_Time(Watchy &watchy) {
  HobbitTime face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
