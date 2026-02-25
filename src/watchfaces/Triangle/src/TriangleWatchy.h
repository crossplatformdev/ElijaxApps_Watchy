#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include "../../../watchy/Watchy.h"
#include "../../../sdk/UiSDK.h"

typedef struct Vector
{
  int x;
  int y;
}Vector;

class TriangleWatchy : public Watchy
{
  using Watchy::Watchy;
public:
  TriangleWatchy();
  void drawWatchFace();
  void drawTime(uint16_t fgColor, uint16_t bgColor);

  int getBatteryFill(int steps);
  
  void drawTriangle(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color);
  void drawTriangleFill(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color);
};

#endif
