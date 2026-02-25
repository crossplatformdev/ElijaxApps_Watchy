#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include "../../../src/watchy/Watchy.h"
#include "../include/images.h"
#include "../include/NIOBRG__10pt7b.h"
#include "../include/NIOBRG__30pt7b.h"

class KaveWatchy : public Watchy
{
  using Watchy::Watchy;
public:
  void drawWatchFace();
  void drawTime();
  void drawDate();
  void drawWeather();
  void drawWeatherIcon(int8_t iconPosX, int16_t iconWeatherConditionCode);
  void drawSteps();
  void drawBattery();
  void drawWifi();

  const char* Ordinal(uint8_t num);

  void drawFillBitmap(int16_t x, int16_t y, int16_t fw, int16_t fh, int16_t ox, int16_t oy, 
    const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
};

#endif
