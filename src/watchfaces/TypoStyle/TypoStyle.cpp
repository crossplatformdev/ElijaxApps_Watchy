#include "TypoStyle.h"
#include "sdk/UiSDK.h"

#define GREY 0x7BEF

namespace {

void drawPixel(Watchy &watchy, uint16_t fgColor, uint16_t bgColor, int16_t x, int16_t y, uint16_t col) {
  switch (col) {
    case GREY:
      UiSDK::drawPixel(watchy.display, x, y, (((x + y) & 1) == 0) ? bgColor : fgColor);
      break;
    default:
      UiSDK::drawPixel(watchy.display, x, y, col);
      break;
  }
}

void drawBitmapCol(Watchy &watchy, uint16_t fgColor, uint16_t bgColor, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color1, uint16_t color2) {
  int16_t i, j, byteWidth = (w + 7) / 8;
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if((pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))) {
        drawPixel(watchy, fgColor, bgColor, x+i, y+j, color1);
      } else {
        drawPixel(watchy, fgColor, bgColor, x+i, y+j, color2);
      }
    }
  }
}

void drawMyRect(Watchy &watchy, uint16_t fgColor, uint16_t bgColor, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color1) {
  for(int j=0; j<h; j++) {
    for(int i=0; i<w; i++ ) {
      drawPixel(watchy, fgColor, bgColor, x+i, y+j, color1);
    }
  }  
}

} // namespace

void showWatchFace_TypoStyle(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  int clockLeft = -14;
  int clockTop = 15;
  int hour1 = watchy.currentTime.Hour/10;
  int hour2 = watchy.currentTime.Hour%10;
  int minute1 = watchy.currentTime.Minute/10;
  int minute2 = watchy.currentTime.Minute%10;
  int month = watchy.currentTime.Month-1;
  int day1 = watchy.currentTime.Day/10;
  int day2 = watchy.currentTime.Day%10;
  int year = watchy.currentTime.Year + 1970;
  int weekday = watchy.currentTime.Wday-1;
  int year1,year2,year3,year4;
  
  year4 = year % 10; year /= 10;
  year3 = year % 10; year /= 10;
  year2 = year % 10; year /= 10;
  year1 = year % 10; year /= 10;

  int col1 = fgColor;
  int col2 = bgColor;

  drawMyRect(watchy, fgColor, bgColor, 0, 0, 200, 200, col2);

  int ds = sizeof(clockDigits)/10;
  int glyphWidth = 54;
  int glyphHeight = 72;
  col1 = fgColor;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft, clockTop, &clockDigits[hour1*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+glyphWidth, clockTop, &clockDigits[hour2*ds], glyphWidth, glyphHeight, col1, col2);
  col1 = GREY;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+(glyphWidth*2)+(glyphWidth*0.25), clockTop, &clockDigits[minute1*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+(glyphWidth*3)+(glyphWidth*0.25), clockTop, &clockDigits[minute2*ds], glyphWidth, glyphHeight, col1, col2);

  drawMyRect(watchy, fgColor, bgColor, clockLeft+(glyphWidth*2)+2, clockTop+20, 10, 10, col1);
  drawMyRect(watchy, fgColor, bgColor, clockLeft+(glyphWidth*2)+2, clockTop+40, 10, 10, col1);
    
  ds = sizeof(monthName)/12;
  col1 = fgColor;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft-9, clockTop+72, &monthName[month*ds], 250, 30, col1, col2);

  ds = sizeof(dateDigits)/10;
  glyphWidth = 45;
  glyphHeight = 60;
  col1 = GREY;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+16,              clockTop+107, &dateDigits[day1*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+16+(glyphWidth), clockTop+107, &dateDigits[day2*ds], glyphWidth, glyphHeight, col1, col2);

  ds = sizeof(dayName)/7;
  col1 = GREY;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+103, clockTop+105, &dayName[weekday*ds], 154, 17, col1, col2);

  ds = sizeof(yearDigits)/10;
  glyphWidth = 30;
  glyphHeight = 45;
  col1 = fgColor;
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+103,                clockTop+122, &yearDigits[year1*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+103+(glyphWidth),   clockTop+122, &yearDigits[year2*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+103+(glyphWidth*2), clockTop+122, &yearDigits[year3*ds], glyphWidth, glyphHeight, col1, col2);
  drawBitmapCol(watchy, fgColor, bgColor, clockLeft+103+(glyphWidth*3), clockTop+122, &yearDigits[year4*ds], glyphWidth, glyphHeight, col1, col2);

  float powerPercent = (watchy.getBatteryVoltage()-3.8)*200;
  drawMyRect(watchy, fgColor, bgColor, 0, 187, (int)powerPercent, 13, GREY);
}
