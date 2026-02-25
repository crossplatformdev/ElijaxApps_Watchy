#ifndef BOTWATCHY_H
#define BOTWATCHY_H

#include "../../../src/watchy/Watchy.h"

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
  void drawTime();

  int getBatteryFill(int steps);
  
  void drawTriangle(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color);
  void drawTriangleFill(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color);
};

#endif
