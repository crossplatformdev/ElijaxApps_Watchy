#ifndef MAIN_H
#define MAIN_H

#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "captnwednesday_img.h"
#include "icons.h"
#include "fonts/Tintin_Dialogue8pt7b.h"
#include "fonts/Tintin_Dialogue9pt7b.h"
#include "fonts/Tintin_Dialogue10pt7b.h"
#include "fonts/Tintin_Dialogue16pt7b.h"

RTC_DATA_ATTR static int face = 0;

const uint8_t BATTERY_SEGMENT_WIDTH = 7;
const uint8_t BATTERY_SEGMENT_HEIGHT = 11;
const uint8_t BATTERY_SEGMENT_SPACING = 9;
const uint8_t WEATHER_ICON_WIDTH = 48;
const uint8_t WEATHER_ICON_HEIGHT = 32;

class WatchyMoto : public Watchy {
  using Watchy::Watchy;
  public:
    void drawWatchFace();
    void drawWrapText(String text);
    void drawBattery();
    void drawSteps();
    void drawCaptnWednesday(float batt);
    virtual void handleButtonPress(); //Must also be virtual in Watchy.h
};

#include "captnwednesday.h"
//#include "faceXXXX.h"

void drawCaptnWednesday(Watchy &watchy, float batt);
void showWatchFace_Captn_Wednesday(Watchy &watchy);

#endif