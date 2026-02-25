#include "Watchy_S2Analog.h"

#include "../../../sdk/UiSDK.h"

RTC_DATA_ATTR static bool displayBattery = false;
RTC_DATA_ATTR static bool dateInFront =    false;

namespace {

void getCosSin(double xyPair[], double deg, unsigned int len)
  { xyPair[0] = cos(deg*=DEG_TO_RAD) * len;  xyPair[1] = sin(deg) * len; }

void getHand(double result[], unsigned int x, unsigned int y, unsigned int deg, unsigned int width1, unsigned int width2, unsigned int len, unsigned int tip)
{
  double xyPair[2]; width1/=2; width2/=2;
  getCosSin(xyPair, (double)deg, len - tip);  double midX = xyPair[0], midY = xyPair[1];  
  getCosSin(xyPair, (double)deg, len);        double tipX = xyPair[0], tipY = xyPair[1];    
  getCosSin(xyPair, (double)deg-90, width1);  double bottomLeftX = xyPair[0], bottomLeftY = xyPair[1];  
  getCosSin(xyPair, (double)deg+90, width1);  double bottomRightX = xyPair[0], bottomRightY = xyPair[1];
  getCosSin(xyPair, (double)deg-90, width2);  double topLeftX = xyPair[0], topLeftY = xyPair[1];  
  getCosSin(xyPair, (double)deg+90, width2);  double topRightX = xyPair[0], topRightY = xyPair[1];

  result[0] = x+bottomLeftX;        result[1] = y+bottomLeftY;
  result[2] = x+(midX+topLeftX);    result[3] = y+(midY+topLeftY);
  result[4] = x+tipX;               result[5] = y+tipY;
  result[6] = x+(midX+topRightX);   result[7] = y+(midY+topRightY);
  result[8] = x+bottomRightX;       result[9] = y+bottomRightY;
}

void drawTicks(Watchy &watchy, uint16_t colour_1)
{
  double deg = 0;
  double xyPair1[2];
  double xyPair2[2];
  double xyPair3[2];
  
  for(int i = 0; i<60; i++)
  {
    if(i%5)
    {
      getCosSin(xyPair1, deg, 87); getCosSin(xyPair2, deg, 94); UiSDK::drawLine(watchy.display, 100+xyPair1[0], 100+xyPair1[1], 100+xyPair2[0], 100+xyPair2[1], colour_1);
    }
    else
    {
      if(i%15)
      {
        getCosSin(xyPair1, deg, 91);
        UiSDK::drawCircle(watchy.display, 100+xyPair1[0], 100+xyPair1[1],  3, colour_1);
      }
      else
      {
        getCosSin(xyPair1, deg, 87); getCosSin(xyPair2, deg-2, 96); getCosSin(xyPair3, deg+2, 96);
        UiSDK::drawTriangle(watchy.display, 100+xyPair1[0], 100+xyPair1[1], 100+xyPair2[0], 100+xyPair2[1], 100+xyPair3[0], 100+xyPair3[1], colour_1);
      }
    }
    deg+=6;    
  };  
}

void drawOverlay(Watchy &watchy, const unsigned int data[], unsigned int osx, unsigned int osy, unsigned int colour)
  { unsigned int rle = 0, len = 0, dy = 0, y = 0; for(int i = 1; i<data[0]; i++){ dy = data[i]; y = rle/200; while(dy>0){ dy-=(len=(dy<200?dy:200)); if((i&1)==0)UiSDK::drawFastHLine(watchy.display, osx+(rle - (y*200)), osy+y, len, colour); rle+=len; }; }; }

void drawTime(Watchy &watchy, uint16_t colour_1, uint16_t colour_2)
{
  UiSDK::drawCircle(watchy.display, 100, 100, 99, colour_1); 
  UiSDK::drawCircle(watchy.display, 100, 100,  84, colour_1);
  UiSDK::drawCircle(watchy.display, 100, 100,  58, colour_1);

  drawTicks(watchy, colour_1);  
  drawOverlay(watchy, s2_numbers, 0, 0, colour_1);

  double hand[10];
  double _step1 = 360/60;
  double _step2 = 360/12;
  double _deg1 = _step1 * watchy.currentTime.Minute;
  double _deg2 = _step2 * watchy.currentTime.Hour;
  
  getHand(hand, 100, 100, _deg1-90, 8, 10, 82, 15);

  UiSDK::fillTriangle(watchy.display, hand[0], hand[1], hand[2], hand[3], hand[8], hand[9], colour_2);
  UiSDK::fillTriangle(watchy.display, hand[8], hand[9], hand[6], hand[7], hand[2], hand[3], colour_2);
  UiSDK::fillTriangle(watchy.display, hand[2], hand[3], hand[4], hand[5], hand[6], hand[7], colour_2);

  UiSDK::drawLine(watchy.display, hand[0], hand[1], hand[2], hand[3], colour_1);
  UiSDK::drawLine(watchy.display, hand[2], hand[3], hand[4], hand[5], colour_1);
  UiSDK::drawLine(watchy.display, hand[4], hand[5], hand[6], hand[7], colour_1);
  UiSDK::drawLine(watchy.display, hand[6], hand[7], hand[8], hand[9], colour_1);
  
  _deg2+=(_step2*((double)watchy.currentTime.Minute/60)); /* nudge it with the minute % */
  getHand(hand, 100, 100, _deg2-90, 12, 15, 70, 10);

  UiSDK::fillTriangle(watchy.display, hand[0], hand[1], hand[2], hand[3], hand[8], hand[9], colour_2);
  UiSDK::fillTriangle(watchy.display, hand[8], hand[9], hand[6], hand[7], hand[2], hand[3], colour_2);
  UiSDK::fillTriangle(watchy.display, hand[2], hand[3], hand[4], hand[5], hand[6], hand[7], colour_2);

  UiSDK::drawLine(watchy.display, hand[0], hand[1], hand[2], hand[3], colour_1);
  UiSDK::drawLine(watchy.display, hand[2], hand[3], hand[4], hand[5], colour_1);
  UiSDK::drawLine(watchy.display, hand[4], hand[5], hand[6], hand[7], colour_1);
  UiSDK::drawLine(watchy.display, hand[6], hand[7], hand[8], hand[9], colour_1);

  UiSDK::fillCircle(watchy.display, 100, 100, 8, colour_2);
  UiSDK::drawCircle(watchy.display, 100, 100, 8, colour_1);   
}

void drawDate(Watchy &watchy, uint16_t colour_1, uint16_t colour_2)
{
    int16_t  x1, y1;
    uint16_t w, h;

    char buf[11];
    char dayOfWeek[4];
    char monthOfYear[4];
    
    strncpy(dayOfWeek, dayStr(watchy.currentTime.Wday), 3); dayOfWeek[3] = 0;
    strncpy(monthOfYear, monthStr(watchy.currentTime.Month), 3); monthOfYear[3] = 0;   
    sprintf(buf, "%s %s%d %s", dayOfWeek, (watchy.currentTime.Day<10?"0":""), watchy.currentTime.Day, monthOfYear);

    UiSDK::setFont(watchy.display, &Seven_Segment10pt7b);
    UiSDK::getTextBounds(watchy.display, buf, 100, 128, &x1, &y1, &w, &h);

    if(dateInFront)
    { 
      UiSDK::fillRect(watchy.display, 57, 112, 87, 21, colour_2);
      UiSDK::drawRect(watchy.display, 57, 112, 87, 21, colour_1);
      UiSDK::setCursor(watchy.display, 99 - (w/2), 128);
      UiSDK::println(watchy.display, buf);
    };
    
    UiSDK::setCursor(watchy.display, 100 - (w/2), 129);
    UiSDK::println(watchy.display, buf); 
}

void drawBattery(Watchy &watchy, uint16_t colour_1, uint16_t colour_2)
{
  float VBAT = watchy.getBatteryVoltage(); 
  int state = max(0, (int)(72*((VBAT-3.80)/0.48)));
  UiSDK::fillRect(watchy.display, 128, 100, state, 100, colour_1);

  for(int x = -200; x<200; x+=2)
    UiSDK::drawLine(watchy.display, x, 0, x+200, 200, colour_2);    

  UiSDK::drawRect(watchy.display, 100, 100, 100, 100, colour_1);
  UiSDK::fillCircle(watchy.display, 100, 100, 102, colour_2);
  UiSDK::drawCircle(watchy.display, 100, 100, 102, colour_1);
  UiSDK::fillRect(watchy.display, 0, 0, 200, 100, colour_2);
  UiSDK::fillRect(watchy.display, 0, 0, 100, 200, colour_2);   
}

} // namespace

void showWatchFace_S2Analog(Watchy &watchy)
{
  UiSDK::initScreen(watchy.display);
  uint16_t colour_1 = UiSDK::getWatchfaceFg(BASE_POLARITY);
  uint16_t colour_2 = UiSDK::getWatchfaceBg(BASE_POLARITY);
  UiSDK::setTextColor(watchy.display, colour_1);

  if(displayBattery)
    drawBattery(watchy, colour_1, colour_2);

  if(dateInFront)
  {      
    drawTime(watchy, colour_1, colour_2);
    drawDate(watchy, colour_1, colour_2);
  }
  else
  {
    drawDate(watchy, colour_1, colour_2);
    drawTime(watchy, colour_1, colour_2);
  };

}
