#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include "../../../watchy/Watchy.h"
#include "../include/images.h"
#include "../include/BLKCHCRY12pt7b.h"
#include "../include/BLKCHCRY40pt7b.h"

class BetonWatchy : public Watchy
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
  const unsigned char* HeartBitmap(int amount);
};

#endif
