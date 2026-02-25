#include "TriangleWatchy.h"

const float VOLTAGE_MIN = 3.4;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

const int TOP_TICK_SIZE = 26;
const int TICK_SIZE = 14;
const int SMALL_TICK = 4;

const Vector TOP_TICK_1 = {-TICK_SIZE / 2, 0};
const Vector TOP_TICK_2 = {TICK_SIZE / 2, 0};
const Vector TOP_TICK_3 = {0, TOP_TICK_SIZE};
const Vector TOP_TICK_4 = {0, TOP_TICK_SIZE / 2};

const Vector TICK_1 = {-TICK_SIZE / 2, 0};
const Vector TICK_2 = {TICK_SIZE / 2, 0};
const Vector TICK_3 = {0, TICK_SIZE};
const Vector TICK_4 = {0, TICK_SIZE / 2};

const Vector SMALL_TICK_1 = {-SMALL_TICK / 2, -SMALL_TICK / 2};
const Vector SMALL_TICK_2 = {SMALL_TICK / 2, SMALL_TICK / 2};
const Vector SMALL_TICK_3 = {-TICK_SIZE, TICK_SIZE};
const Vector SMALL_TICK_4 = {-TICK_SIZE / 2, TICK_SIZE / 2};

const Vector SMALL_TICK_POS_1 = {48,-84};
const Vector SMALL_TICK_POS_2 = {84,-48};

const int PIN_SIZE = 4;

const Vector PIN_1 = {-PIN_SIZE / 2, PIN_SIZE / 2};
const Vector PIN_2 = {PIN_SIZE / 2, PIN_SIZE / 2};
const Vector PIN_3 = {0, -PIN_SIZE / 2};

const int LENGTH_BACK_OFFSET = 10;
const int LENGTH_FORWARD_OFFSET = 18;
const int HOUR_HAND_WIDTH = 24;
const int HOUR_HAND_LENGTH = 35;

const Vector HOUR_HAND_1 = {-HOUR_HAND_WIDTH / 2, LENGTH_BACK_OFFSET};
const Vector HOUR_HAND_2 = {HOUR_HAND_WIDTH / 2, LENGTH_BACK_OFFSET};
const Vector HOUR_HAND_3 = {0, -HOUR_HAND_LENGTH - LENGTH_FORWARD_OFFSET};
const Vector HOUR_HAND_4 = {0, -HOUR_HAND_LENGTH - LENGTH_FORWARD_OFFSET + 25};

const int MINUTE_HAND_WIDTH = 12;
const int MINUTE_HAND_LENGTH = 75;
const int BORDER_WIDTH = 2;

const Vector MINUTE_HAND_START = {0, -HOUR_HAND_LENGTH};
const Vector MINUTE_HAND_1 = {-MINUTE_HAND_WIDTH / 2, LENGTH_BACK_OFFSET};
const Vector MINUTE_HAND_2 = {MINUTE_HAND_WIDTH / 2, LENGTH_BACK_OFFSET};
const Vector MINUTE_HAND_3 = {0, -MINUTE_HAND_LENGTH};
const Vector MINUTE_HAND_4 = {0, -MINUTE_HAND_LENGTH + 25};

const int BATTERY_LENGTH = MINUTE_HAND_1.y - MINUTE_HAND_4.y;

const double STEP_MINUTE = 360/60;
const double STEP_HOUR = 360/12;

const Vector CORNER_1 = {33, 99};
const Vector CORNER_2 = {99, 33};
const Vector CORNER_3 = {99, 99};
const Vector CORNER_4 = {80, 80};

void TriangleWatchy::drawWatchFace()
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  drawTime();
}

static Vector rotateVector(Vector vector, double angle)
{
  double radians = angle * DEG_TO_RAD;
  double sinAngle = sin(radians);
  double cosAngle = cos(radians);

  Vector newVector = {0,0};
  newVector.x = cosAngle * (double)vector.x - sinAngle * (double)vector.y;
  newVector.y = sinAngle * (double)vector.x + cosAngle * (double)vector.y;
  return newVector;
}

static double dotProduct(Vector v1, Vector v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

static double crossProduct(Vector v1, Vector v2)
{
  return v1.x * v2.y - v1.y * v2.x;
}

static Vector rotateVectorByRightAngle(Vector vector, int angle)
{
  angle = angle % 4;

  switch (angle)
  {
    case 1:
      return {-vector.y, vector.x};
    case 2:
      return {-vector.x, -vector.y};
    case 3:
      return {vector.y, -vector.x};
    default:
      return vector;
  }
}

int TriangleWatchy::getBatteryFill(int steps)
{
  float VBAT = getBatteryVoltage();

  // 12 battery states
  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * steps);
  if (batState > steps)
    batState = steps;
  if (batState < 0)
    batState = 0;

  return batState;
}

void TriangleWatchy::drawTriangle(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color)
{
  display.drawTriangle(center.x + v1.x, center.y + v1.y, center.x + v2.x, center.y + v2.y, center.x + v3.x, center.y + v3.y, color);
}

void TriangleWatchy::drawTriangleFill(Vector v1, Vector v2, Vector v3, Vector center, uint16_t color)
{
  display.fillTriangle(center.x + v1.x, center.y + v1.y, center.x + v2.x, center.y + v2.y, center.x + v3.x, center.y + v3.y, color);
}

static Vector flip(Vector vector, bool negativeX, bool negativeY)
{
  if (negativeX)
    vector.x *= -1;

  if (negativeY)
    vector.y *= -1;

  return vector;
}

void TriangleWatchy::drawTime()
{
  int hour = currentTime.Hour;
  int minute = currentTime.Minute;

  drawTriangleFill(TOP_TICK_1, TOP_TICK_2, TOP_TICK_3, {100, 1}, GxEPD_BLACK);
  drawTriangleFill(TOP_TICK_1, TOP_TICK_2, TOP_TICK_4, {100, 1}, GxEPD_WHITE);
  drawTriangle(TOP_TICK_1, TOP_TICK_2, TOP_TICK_3, {100, 1}, GxEPD_BLACK);

  for (int i = 0; i < 4; i++)
  {    
    bool negativeX = i > 1;
    bool negativeY = i > 0 && i < 3;

    Vector corner1 = flip(CORNER_1, negativeX, negativeY);
    Vector corner2 = flip(CORNER_2, negativeX, negativeY);
    Vector corner3 = flip(CORNER_3, negativeX, negativeY);
    Vector corner4 = flip(CORNER_4, negativeX, negativeY);

    drawTriangleFill(corner1, corner2, corner3, {100,100}, GxEPD_BLACK);
    drawTriangleFill(corner1, corner2, corner4, {100,100}, GxEPD_WHITE);
    drawTriangle(corner1, corner2, corner3, {100,100}, GxEPD_BLACK);
  }

  for (int i = 0; i < 4; i++)
  {    
    Vector tick1 = rotateVectorByRightAngle(SMALL_TICK_1, i);
    Vector tick2 = rotateVectorByRightAngle(SMALL_TICK_2, i);
    Vector tick3 = rotateVectorByRightAngle(SMALL_TICK_3, i);
    Vector tick4 = rotateVectorByRightAngle(SMALL_TICK_4, i);
    
    Vector tickCenter1 = rotateVectorByRightAngle(SMALL_TICK_POS_1, i);
    tickCenter1.x += 100;
    tickCenter1.y += 100;
    
    Vector tickCenter2 = rotateVectorByRightAngle(SMALL_TICK_POS_2, i);
    tickCenter2.x += 100;
    tickCenter2.y += 100;

    drawTriangleFill(tick1, tick2, tick3, tickCenter1, GxEPD_BLACK);
    drawTriangleFill(tick1, tick2, tick4, tickCenter1, GxEPD_WHITE);
    drawTriangle(tick1, tick2, tick3, tickCenter1, GxEPD_BLACK);

    drawTriangleFill(tick1, tick2, tick3, tickCenter2, GxEPD_BLACK);
    drawTriangleFill(tick1, tick2, tick4, tickCenter2, GxEPD_WHITE);
    drawTriangle(tick1, tick2, tick3, tickCenter2, GxEPD_BLACK);
  }

  for (int i = 1; i < 4; i++)
  {    
    Vector tick1 = rotateVectorByRightAngle(TICK_1, i);
    Vector tick2 = rotateVectorByRightAngle(TICK_2, i);
    Vector tick3 = rotateVectorByRightAngle(TICK_3, i);
    Vector tick4 = rotateVectorByRightAngle(TICK_4, i);
    
    Vector tickCenter = rotateVectorByRightAngle({0, -99}, i);
    tickCenter.x += 100;
    tickCenter.y += 100;

    drawTriangleFill(tick1, tick2, tick3, tickCenter, GxEPD_BLACK);
    drawTriangleFill(tick1, tick2, tick4, tickCenter, GxEPD_WHITE);
    drawTriangle(tick1, tick2, tick3, tickCenter, GxEPD_BLACK);
  }

  double degHour = STEP_HOUR * ((double)hour + minute/60.0);

  Vector hour1 = rotateVector(HOUR_HAND_1, degHour);
  Vector hour2 = rotateVector(HOUR_HAND_2, degHour);
  Vector hour3 = rotateVector(HOUR_HAND_3, degHour);
  Vector hour4 = rotateVector(HOUR_HAND_4, degHour);
  Vector center = {100, 100};

  drawTriangleFill(hour1, hour2, hour3, center, GxEPD_BLACK);
  drawTriangleFill(hour1, hour2, hour4, center, GxEPD_WHITE);
  drawTriangle(hour1, hour2, hour3, center, GxEPD_BLACK);

  drawTriangleFill(PIN_1, PIN_2, PIN_3, center, GxEPD_BLACK);

  double degMinute = STEP_MINUTE * minute;
  
  Vector minuteStart = rotateVector(MINUTE_HAND_START, degHour);
  
  // minuteStart = center;

  // Calculate correct angle so offsetted minute hand point at current minute
  Vector minuteTarget = rotateVector({0,-99}, degMinute);
  Vector minuteVector = {minuteTarget.x - minuteStart.x, minuteTarget.y - minuteStart.y};
  
  degMinute = atan2(crossProduct({0,-1}, minuteVector), dotProduct({0,-1}, minuteVector)) * RAD_TO_DEG;

  Vector minute1 = rotateVector(MINUTE_HAND_1, degMinute);
  Vector minute2 = rotateVector(MINUTE_HAND_2, degMinute);
  Vector minute3 = rotateVector(MINUTE_HAND_3, degMinute);

  minuteStart.x = 100 + minuteStart.x;
  minuteStart.y = 100 + minuteStart.y;
  
  drawTriangleFill(minute1, minute2, minute3, minuteStart, GxEPD_BLACK);

  Vector battery1 = rotateVector(MINUTE_HAND_1, degMinute);
  Vector battery2 = rotateVector(MINUTE_HAND_2, degMinute);

  int battery = getBatteryFill(BATTERY_LENGTH);

  Vector batteryFill = {MINUTE_HAND_4.x, MINUTE_HAND_4.y};
  batteryFill.y += BATTERY_LENGTH - battery;

  Vector battery3 = rotateVector(batteryFill, degMinute);

  drawTriangleFill(battery1, battery2, battery3, minuteStart, GxEPD_WHITE);
  
  drawTriangle(minute1, minute2, minute3, minuteStart, GxEPD_BLACK);
  
  drawTriangleFill(PIN_1, PIN_2, PIN_3, minuteStart, GxEPD_BLACK);

  display.drawRect(1,1,199,199, GxEPD_BLACK);
}
