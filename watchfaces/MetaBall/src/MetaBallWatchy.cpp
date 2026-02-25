#include "MetaBallWatchy.h"
#include <VectorXf.h>

const float VOLTAGE_MIN = 3.5;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_WARNING = 3.6;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

MetaBallWatchy::MetaBallWatchy(const watchySettings& s) : Watchy(s)
{
  //Serial.begin(115200);
//while (!Serial); // wait for serial port to connect. Needed for native USB port on Arduino only

}

static bool getColor(const int16_t& x, const int16_t& y, const uint16_t& color) 
{
  return color > BlueNoise200[y * 200 + x];
}

static bool getColor2(int16_t x, int16_t y, int16_t xUv, int16_t yUv, const uint8_t *bitmap, int16_t w, int16_t h) 
{
  return getColor(x, y, bitmap[yUv * w + xUv]);
}

static bool getColor3(const int16_t& x, const int16_t& y, const int16_t& xUv, const int16_t& yUv, const uint8_t *bitmap, const int16_t& w, const int16_t& h) 
{
  return getColor(x,y,bitmap[yUv * w + xUv]);
}

static float clamp(float val, const float& min, const float& max)
{
  if (val > max)
    val = max;
  
  if (val < min)
    val = min;

  return val;
}

float MetaBallWatchy::getBatteryFill()
{
  float VBAT = getBatteryVoltage();

  float batState = ((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE);

  return clamp(batState, 0.0f, 1.0f);
}

static float unlerp(const float& value, const float& min, const float& max) {
  return (value - min) / (max - min);
}

static float smoothstep(const float& x) {
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}

static Vec2f ClosestPointOnSegment(const Vec2f& s1, const Vec2f& s2, const Vec2f& p, float& unlerp)
{
  Vec2f difference = s2 - s1;
  float sqrMagnitude = difference.lengthSquared();

  if (sqrMagnitude > 0.0f)
    unlerp = (p - s1).dot(difference) / sqrMagnitude;
  else
    unlerp = 0.0f;

  unlerp = clamp(unlerp, 0.0f, 1.0f);
  return s1 + difference * unlerp;
}

static Vec2f ClosestPointOnSegment(const Vec2f& s1, const Vec2f& s2, const Vec2f& p)
{
  float unlerp = 0.0f;
  return ClosestPointOnSegment(s1, s2, p, unlerp);
}

static Vec2f getPerpendicular(Vec2f vector) {
	return Vec2f( -vector.y, vector.x );
}

static Vec2f ClosestPointOnArc(const Vec2f& center, const Vec2f& start, const Vec2f& end, const float& radius, const float& angleStart, const float& angle, const Vec2f& p)
{
  Vec2f diff = p - center;

  bool over180 = angle > 180;

  Vec2f startPerpendicular = getPerpendicular(start);
  Vec2f endPerpendicular = -getPerpendicular(end);
  float startPrependicularDot = startPerpendicular.dot(diff);
  float endPrependicularDot = endPerpendicular.dot(diff);

  bool isPointOnArc = !over180 && (startPrependicularDot >= 0.0f && endPrependicularDot >= 0.0f) || over180 && !(startPrependicularDot < 0.0f && endPrependicularDot < 0.0f);

  if (isPointOnArc)
    return center + diff.getNormalized() * radius;

  if (startPrependicularDot < endPrependicularDot)
    return center + end * radius;
  
  return center + start * radius;
}

static void MetaBall(const Vec2f& currentPos, const Vec2f& circleCenter, const float& radius, const float& extraRadius, int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  Vec2f offset = currentPos - circleCenter;
  float totalRadius = radius + extraRadius;
  float lengthSqr = offset.lengthSquared();

  if (lengthSqr >= totalRadius * totalRadius)
    return;

  float radiusSqr = radius * radius;
  float newDistance = 0.0f;
  
  count++;

  if (lengthSqr < radiusSqr)
  {
    if (count > 1)
      newDistance = sqrt(radiusSqr - lengthSqr) + extraRadius;
    else
      newDistance = 1.0f + extraRadius;
  }
  else
  {
    float distance = sqrtf(lengthSqr);
    newDistance = totalRadius - distance;
    newDistance = smoothstep(unlerp(newDistance, 0, extraRadius) * 0.5f) * 2.0f * extraRadius;
  }

  if (count == 1)
  {
    prevRadius = radius;
    prevCenter = circleCenter;
    totalDistance = newDistance;
    return;
  }
  else if (count == 2)
  {
    Vec2f offsetPrev = currentPos - prevCenter;
    float lengthPrevSqr = offsetPrev.lengthSquared();
  
    float radiusPrevSqr = prevRadius * prevRadius;

    if (lengthPrevSqr < radiusPrevSqr)
    {
      totalDistance = sqrt(radiusPrevSqr - lengthPrevSqr) + extraRadius;
    }
  }

  Vec2f offsetBetweenCenters = circleCenter - prevCenter;
  Vec2f offsetBetweenCentersNormalized = offsetBetweenCenters.getNormalized();

  float weight = 0.0f;
  ClosestPointOnSegment(circleCenter - offsetBetweenCentersNormalized * totalRadius, prevCenter + offsetBetweenCentersNormalized * (prevRadius + extraRadius), currentPos, weight);
  
  float otherWeight = 1.0f - weight;
  Vec2f newCenter = circleCenter * weight + prevCenter * otherWeight;
  prevCenter = newCenter;
  
  totalDistance += newDistance;
  Vec3f pointOnSurface (currentPos.x, currentPos.y, totalDistance);
  Vec3f centerV3 (newCenter.x, newCenter.y, extraRadius);
  
  prevRadius = (pointOnSurface - centerV3).length();
}

static void Arc(const Vec2f& currentPos, const Vec2f& center, const Vec2f& start, const Vec2f& end, const float& radius, const float& extraRadius,
 const float& arcRadius, const float& arcStartAngle, const float& arcAngle,
  int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  Vec2f offset = currentPos - center;
  float outterRadius = radius + extraRadius + arcRadius;

  float lengthSquared = offset.lengthSquared();

  if (lengthSquared >= outterRadius * outterRadius)
    return;
    
  float innerRadius = arcRadius - radius - extraRadius;

  if (innerRadius > 0.0f && lengthSquared <= innerRadius * innerRadius)
    return;

  Vec2f closestToArc = ClosestPointOnArc(center, start, end, arcRadius, arcStartAngle, arcAngle, currentPos);
  MetaBall(currentPos, closestToArc, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

const float RADIUS_PERCENTAGE = 0.2f;
const float HALF_OFFSET = (0.5f - RADIUS_PERCENTAGE);
const float OFFSET = (1.0f - RADIUS_PERCENTAGE);

static void Draw0(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
 int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * 0.5f;
  Vec2f s1 = center + Vec2f(0.0f, radius);
  Vec2f s2 = center + Vec2f(0.0f, -radius);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

static void Draw1(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(size * HALF_OFFSET, -size * OFFSET);
  Vec2f s2 = center + Vec2f(size * HALF_OFFSET, size * OFFSET);
  Vec2f s3 = center + Vec2f(-size * HALF_OFFSET, -size * 0.2f);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  closestPoint = ClosestPointOnSegment(s1, s3, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

const float DRAW_2_ARC_START_ANGLE = 180.0f;
const Vec2f DRAW_2_ARC_START = Vec2f(cos(DRAW_2_ARC_START_ANGLE * DEG_TO_RAD), sin(DRAW_2_ARC_START_ANGLE * DEG_TO_RAD));

const float DRAW_2_ARC_END_ANGLE = 198.0f;
const float DRAW_2_ARC_END_ANGLE_SUM = DRAW_2_ARC_START_ANGLE + DRAW_2_ARC_END_ANGLE;
const Vec2f DRAW_2_ARC_END = Vec2f(cos(DRAW_2_ARC_END_ANGLE_SUM * DEG_TO_RAD), sin(DRAW_2_ARC_END_ANGLE_SUM * DEG_TO_RAD));

static void Draw2(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f arcCenter = center + Vec2f(0.0f, -size * 0.5f);
  Vec2f s2 = center + Vec2f(size * 0.2f, -size * 0.2f);
  Vec2f s3 = center + Vec2f(-size * HALF_OFFSET, size * OFFSET);
  Vec2f s4 = center + Vec2f(size * HALF_OFFSET, size * OFFSET);

  if (currentPos.y < arcCenter.y + DRAW_2_ARC_END.y * size * HALF_OFFSET + radius + extraRadius)
    Arc(currentPos, arcCenter, DRAW_2_ARC_START, DRAW_2_ARC_END, radius, extraRadius, size * HALF_OFFSET,
    DRAW_2_ARC_START_ANGLE, DRAW_2_ARC_END_ANGLE, count, totalDistance, prevRadius, prevCenter);

  Vec2f closestPoint = ClosestPointOnSegment(s2, s3, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  closestPoint = ClosestPointOnSegment(s3, s4, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}


const float DRAW_3_ARC_START_ANGLE = 240.0f;
const Vec2f DRAW_3_ARC_START = Vec2f(cos(DRAW_3_ARC_START_ANGLE * DEG_TO_RAD), sin(DRAW_3_ARC_START_ANGLE * DEG_TO_RAD));

const float DRAW_3_ARC_END_ANGLE = 200.0f;
const float DRAW_3_ARC_END_ANGLE_SUM = DRAW_3_ARC_START_ANGLE + DRAW_3_ARC_END_ANGLE;
const Vec2f DRAW_3_ARC_END = Vec2f(cos(DRAW_3_ARC_END_ANGLE_SUM * DEG_TO_RAD), sin(DRAW_3_ARC_END_ANGLE_SUM * DEG_TO_RAD));

const float DRAW_3_2_ARC_START_ANGLE = 280.0f;
const Vec2f DRAW_3_2_ARC_START = Vec2f(cos(DRAW_3_2_ARC_START_ANGLE * DEG_TO_RAD), sin(DRAW_3_2_ARC_START_ANGLE * DEG_TO_RAD));

const float DRAW_3_2_ARC_END_ANGLE = 200.0f;
const float DRAW_3_2_ARC_END_ANGLE_SUM = DRAW_3_2_ARC_START_ANGLE + DRAW_3_2_ARC_END_ANGLE;
const Vec2f DRAW_3_2_ARC_END = Vec2f(cos(DRAW_3_2_ARC_END_ANGLE_SUM * DEG_TO_RAD), sin(DRAW_3_2_ARC_END_ANGLE_SUM * DEG_TO_RAD));

static void Draw3(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  
  Arc(currentPos, center + Vec2f(-size * 0.1f, -size * 0.4f), DRAW_3_ARC_START, DRAW_3_ARC_END, radius, extraRadius, size * (HALF_OFFSET + 0.1f),
   DRAW_3_ARC_START_ANGLE, DRAW_3_ARC_END_ANGLE, count, totalDistance, prevRadius, prevCenter);
  
  Arc(currentPos, center + Vec2f(-size * 0.1f, size * 0.4f), DRAW_3_2_ARC_START, DRAW_3_2_ARC_END, radius, extraRadius, size * (HALF_OFFSET + 0.1f),
   DRAW_3_2_ARC_START_ANGLE, DRAW_3_2_ARC_END_ANGLE, count, totalDistance, prevRadius, prevCenter);
}

static void Draw4(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(0.0f, -size * OFFSET);
  Vec2f s2 = center + Vec2f(-size * HALF_OFFSET, size * 0.2f);
  Vec2f s3 = center + Vec2f(size * HALF_OFFSET, size * 0.2f);
  Vec2f s4 = center + Vec2f(size * 0.3f, 0.0f);
  Vec2f s5 = center + Vec2f(size * 0.1f, size * OFFSET);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);

  closestPoint = ClosestPointOnSegment(s2, s3, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);

  closestPoint = ClosestPointOnSegment(s4, s5, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

const float DRAW_5_ARC_START_ANGLE = 235.0f;
const Vec2f DRAW_5_ARC_START = Vec2f(cos(DRAW_5_ARC_START_ANGLE * DEG_TO_RAD), sin(DRAW_5_ARC_START_ANGLE * DEG_TO_RAD));

const float DRAW_5_ARC_END_ANGLE = 250.0f;
const float DRAW_5_ARC_END_ANGLE_SUM = DRAW_5_ARC_START_ANGLE + DRAW_5_ARC_END_ANGLE;
const Vec2f DRAW_5_ARC_END = Vec2f(cos(DRAW_5_ARC_END_ANGLE_SUM * DEG_TO_RAD), sin(DRAW_5_ARC_END_ANGLE_SUM * DEG_TO_RAD));

static void Draw5(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(size * HALF_OFFSET, -size * OFFSET);
  Vec2f s2 = center + Vec2f(-size * HALF_OFFSET, -size * OFFSET);
  Vec2f s3 = center + Vec2f(-size * HALF_OFFSET, 0.0f);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  closestPoint = ClosestPointOnSegment(s2, s3, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);

  Arc(currentPos, center + Vec2f(-size * 0.1f, size * 0.4f), DRAW_5_ARC_START, DRAW_5_ARC_END, radius, extraRadius, size * (HALF_OFFSET + 0.1f),
   DRAW_5_ARC_START_ANGLE, DRAW_5_ARC_END_ANGLE, count, totalDistance, prevRadius, prevCenter);
}

static void Draw6(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(size * HALF_OFFSET, -size * OFFSET);
  Vec2f s2 = center + Vec2f(-size * HALF_OFFSET * 0.5f, size * 0.35f * 0.4f);
  Vec2f s3 = center + Vec2f(0.0f, size * 0.5f);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  MetaBall(currentPos, s3, size * 0.5f, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

static void Draw7(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(size * HALF_OFFSET, -size * OFFSET);
  Vec2f s2 = center + Vec2f(-size * HALF_OFFSET, size * OFFSET);
  Vec2f s3 = center + Vec2f(-size * HALF_OFFSET, -size * OFFSET);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  closestPoint = ClosestPointOnSegment(s1, s3, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

static void Draw8(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  Vec2f s1 = center + Vec2f(0.0f, -size * 0.6f);
  Vec2f s2 = center + Vec2f(0.0f, size * 0.5f);
  Vec2f s3 = center + Vec2f(0.0f, size * 0.2f);

  MetaBall(currentPos, s1, size * 0.4f, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  Vec2f closestPoint = ClosestPointOnSegment(s2, s3, currentPos);
  MetaBall(currentPos, closestPoint, size * 0.5f, extraRadius, count, totalDistance,prevRadius, prevCenter);
}

static void Draw9(const Vec2f& currentPos, const Vec2f& center, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  float radius = size * RADIUS_PERCENTAGE;
  Vec2f s1 = center + Vec2f(-size * HALF_OFFSET, size * OFFSET);
  Vec2f s2 = center + Vec2f(size * HALF_OFFSET * 0.5f, -size * 0.35f * 0.4f);
  Vec2f s3 = center + Vec2f(0.0f, -size * 0.5f);

  Vec2f closestPoint = ClosestPointOnSegment(s1, s2, currentPos);
  MetaBall(currentPos, closestPoint, radius, extraRadius, count, totalDistance, prevRadius, prevCenter);
  
  MetaBall(currentPos, s3, size * 0.5f, extraRadius, count, totalDistance, prevRadius, prevCenter);
}

static void DrawDigit(const Vec2f& currentPos, const Vec2f& center, const int& digit, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  switch (digit)
  {
  case 0:
    Draw0(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 1:
    Draw1(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 2:
    Draw2(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 3:
    Draw3(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 4:
    Draw4(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 5:
    Draw5(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 6:
    Draw6(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 7:
    Draw7(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 8:
    Draw8(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
    
  case 9:
    Draw9(currentPos, center, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
    break;
  
  default:
    break;
  }
}

const float NUMBER_SEPARATION = 4.0f;
const float NUMBER_SEPARATION_HALF = NUMBER_SEPARATION * 0.5f;

static void DrawNumber(const Vec2f& currentPos, const Vec2f& center, const int& number, const float& size, const float& extraRadius,
int& count, float& totalDistance, float& prevRadius, Vec2f& prevCenter)
{
  if (currentPos.x <= center.x + extraRadius)
  {
    int firstDigit = number / 10;
    DrawDigit(currentPos, center + Vec2f(-size * 0.5f - NUMBER_SEPARATION_HALF, 0.0f), firstDigit, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
  }

  if (currentPos.x >= center.x - extraRadius)
  {
    int secondDigit = number % 10;
    DrawDigit(currentPos, center + Vec2f(size * 0.5f + NUMBER_SEPARATION_HALF, 0.0f), secondDigit, size, extraRadius, count, totalDistance, prevRadius, prevCenter);
  }
}

const float EXTRA_RADIUS = 9.0f;
const float NUMBER_SIZE = 42.0f;

const float COLON_RADIUS = 6.0f;
const float COLON_OFFSET = 9.0f;

const float HORIZONTAL_CENTER = 99.0f;

const float SLASH_RADIUS = 6.0f;
const float SLASH_WIDTH = 10.0f;
const float SLASH_WIDTH_HALF = SLASH_WIDTH * 0.5f;

const float BATTERY_Y_POS = 99.0f;
const float BATTERY_RADIUS = 5.0f;

const float TOP_LINE = BATTERY_Y_POS - BATTERY_RADIUS - NUMBER_SIZE - EXTRA_RADIUS + 3.0f;
const float BOTTOM_LINE = BATTERY_Y_POS + BATTERY_RADIUS + NUMBER_SIZE + EXTRA_RADIUS - 3.0f;

const float LEFT_LINE = HORIZONTAL_CENTER - COLON_RADIUS - NUMBER_SIZE - NUMBER_SEPARATION_HALF - 5.0f;
const float RIGHT_LINE = HORIZONTAL_CENTER + COLON_RADIUS + NUMBER_SIZE + NUMBER_SEPARATION_HALF + 5.0f;

const Vec2f TOP_LEFT_POINT = Vec2f(LEFT_LINE, TOP_LINE);
const Vec2f TOP_RIGHT_POINT = Vec2f(RIGHT_LINE, TOP_LINE);
const Vec2f BOTTOM_LEFT_POINT = Vec2f(LEFT_LINE, BOTTOM_LINE);
const Vec2f BOTTOM_RIGHT_POINT = Vec2f(RIGHT_LINE, BOTTOM_LINE);

const Vec2f COLON_TOP = Vec2f(HORIZONTAL_CENTER, TOP_LINE - COLON_OFFSET);
const Vec2f COLON_BOTTOM = Vec2f(HORIZONTAL_CENTER, TOP_LINE + COLON_OFFSET);

const Vec2f SLASH_TOP = Vec2f(HORIZONTAL_CENTER + SLASH_WIDTH_HALF, BOTTOM_LINE - NUMBER_SIZE + SLASH_RADIUS);
const Vec2f SLASH_BOTTOM  = Vec2f(HORIZONTAL_CENTER - SLASH_WIDTH_HALF, BOTTOM_LINE + NUMBER_SIZE - SLASH_RADIUS);
const Vec2f CENTER = Vec2f(100.0f, 100.0f);

void MetaBallWatchy::drawWatchFace()
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  const float battery = getBatteryFill();

  const int hour = currentTime.Hour;
  
  const int minute = currentTime.Minute;

  const int month = currentTime.Month;

  const int day = currentTime.Day;

  const float batteryHalfSize = (100.0f - BATTERY_RADIUS - 1.0f) * battery;
  const Vec2f batteryPointLeft (HORIZONTAL_CENTER - batteryHalfSize, BATTERY_Y_POS);
  const Vec2f batteryPointRight (HORIZONTAL_CENTER + batteryHalfSize, BATTERY_Y_POS);

  for (int y = 0; y < 200; ++y)
  {
    for (int x = 0; x < 200; ++x)
    {
      Vec2f currentPos ((float)x, (float)y);

      float totalDistance = 0.0f;
      int count = 0;

      float prevRadius = 0.0f;
      Vec2f center = currentPos;

      if (y <= TOP_LINE + NUMBER_SIZE + EXTRA_RADIUS)
      {
        if (x < LEFT_LINE + NUMBER_SIZE + NUMBER_SEPARATION_HALF + EXTRA_RADIUS)
        {
          DrawNumber(currentPos, TOP_LEFT_POINT, hour, NUMBER_SIZE, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }

        if (x > RIGHT_LINE - NUMBER_SIZE - NUMBER_SEPARATION_HALF - EXTRA_RADIUS)
        {
          DrawNumber(currentPos, TOP_RIGHT_POINT, minute, NUMBER_SIZE, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }
        
        if (x >= HORIZONTAL_CENTER - COLON_RADIUS - EXTRA_RADIUS && x <= HORIZONTAL_CENTER + COLON_RADIUS + EXTRA_RADIUS)
        {
          MetaBall(currentPos, COLON_TOP, COLON_RADIUS, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
          MetaBall(currentPos, COLON_BOTTOM, COLON_RADIUS, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }
      }

      if (y >= BOTTOM_LINE - NUMBER_SIZE - EXTRA_RADIUS)
      {
        if (x < LEFT_LINE + NUMBER_SIZE + NUMBER_SEPARATION_HALF + EXTRA_RADIUS)
        {
          DrawNumber(currentPos, BOTTOM_LEFT_POINT, month, NUMBER_SIZE, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }

        if (x > RIGHT_LINE - NUMBER_SIZE - NUMBER_SEPARATION_HALF - EXTRA_RADIUS)
        {
          DrawNumber(currentPos, BOTTOM_RIGHT_POINT, day, NUMBER_SIZE, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }

        if (x > HORIZONTAL_CENTER - SLASH_WIDTH_HALF - SLASH_RADIUS - EXTRA_RADIUS && x < HORIZONTAL_CENTER + SLASH_WIDTH_HALF + SLASH_RADIUS + EXTRA_RADIUS)
        {
          Vec2f closestPoint = ClosestPointOnSegment(SLASH_TOP, SLASH_BOTTOM, currentPos);
          MetaBall(currentPos, closestPoint, SLASH_RADIUS, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
        }
      }

      if (y >= BATTERY_Y_POS - BATTERY_RADIUS - EXTRA_RADIUS && y <= BATTERY_Y_POS + BATTERY_RADIUS + EXTRA_RADIUS &&
        x >= batteryPointLeft.x - BATTERY_RADIUS - EXTRA_RADIUS && x <= batteryPointRight.x + BATTERY_RADIUS + EXTRA_RADIUS)
      {
        Vec2f closestPoint = ClosestPointOnSegment(batteryPointLeft, batteryPointRight, currentPos);
        MetaBall(currentPos, closestPoint, BATTERY_RADIUS, EXTRA_RADIUS, count, totalDistance, prevRadius, center);
      }

      if (count > 0)
      { 
        if (totalDistance > EXTRA_RADIUS)
        {
          Vec2f normal = (currentPos - center);
          normal *= 100.0f / prevRadius;
          normal += CENTER;
          display.drawPixel(x, y, getColor3(x, y, normal.x, normal.y, MatCapSource, 200,200));
        }
        else if (totalDistance >= EXTRA_RADIUS - 1.0f)
          display.drawPixel(x,y,GxEPD_BLACK);
      }
    }
  }
}
