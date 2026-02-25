#include "ShadowClockWatchy.h"

const float VOLTAGE_MIN = 3.5f;
const float VOLTAGE_MAX = 4.2f;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

const float MINUTE_ANGLE = 360 / 60;
const float HOUR_ANGLE = 360 / 12;

const float CONE_RADIUS = 10;
const float CONE_RADIUS_SQR = CONE_RADIUS * CONE_RADIUS;
const float CONE_HEIGHT = 50;

const float WATCH_RADIUS = 98;
const float WATCH_RADIUS_SQR = WATCH_RADIUS * WATCH_RADIUS;

const float WATCH_RIM_RADIUS = 93;
const float WATCH_RIM_RADIUS_SQR = WATCH_RIM_RADIUS * WATCH_RIM_RADIUS;

const float NORMAL_SCALE = 2.0f / 255.0f;

const Vector3<float> UP = {0.0f, -1.0f, 0.0f};

const Vector3<float> CONE_CENTER = {99.5f, 99.5f, 0.0f};
const Vector3<float> NORMAL_DIR = {0.0f, 0.0f, 1.0f};
const Vector3<float> CONE_TOP = CONE_CENTER + NORMAL_DIR * CONE_HEIGHT;
const Vector3<float> VIEW_DIR = {0.0f, 0.0f, 1.0f};

const Vector3<float> MINUTE_LIGHT = {0.0f,-0.75f,0.6f};
const Vector3<float> MINUTE_LIGHT_LOW = {0.0f,-1.2f,0.6f};

const Vector3<float> HOUR_LIGHT = {0.0f,-0.5f,0.6f};
const Vector3<float> HOUR_LIGHT_LOW = {0.0f,-0.75f,0.6f};

void ShadowClockWatchy::drawWatchFace()
{
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);

  drawTime();
}

int ShadowClockWatchy::getBatteryFill(int steps)
{
  float vbat = getBatteryVoltage();

  // 12 battery states
  int batState = int(((vbat - VOLTAGE_MIN) / VOLTAGE_RANGE) * steps);
  if (batState > steps)
    batState = steps;
  if (batState < 0)
    batState = 0;

  return batState;
}

static float clamp(const float& value)
{
  float outValue = value;
  if (outValue > 1.0f)
        outValue = 1.0f;
  else if (outValue < 0.0f)
        outValue = 0.0f;

  return outValue;
}

static float sign (const Vector3<float>& p1, const Vector3<float>& p2, const Vector3<float>& p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

static bool PointInTriangle (const Vector3<float>& pt, const Vector3<float>& v1, const Vector3<float>& v2, const Vector3<float>& v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);

    if (!has_neg)
      return true;

    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !has_pos;
}

static float smoothstep(float x) {
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}

static void BlinnPhong(const Vector3<float>& point, const float& pointFromCenterSqrMagnitude, const Vector3<float>& normal, const Vector3<float>& lightDir, 
const Vector3<float>& halfView, const char& gloss, 
const Vector3<float>& coneBase1, const Vector3<float>& coneBase2, const Vector3<float>& coneBase3, float& intensity, float& specularIntensity)
{
  if (pointFromCenterSqrMagnitude > CONE_RADIUS_SQR && PointInTriangle(point, coneBase1, coneBase2, coneBase3))
    return; // Point in shadow

  float NdotL = Vector3<float>::dotProduct(normal, lightDir);
  intensity += smoothstep(clamp(NdotL));

  float NdotH = Vector3<float>::dotProduct(normal, halfView);
  specularIntensity += GlossPow[(int)(clamp(NdotH) * 255) + 256 * gloss];
}

void ShadowClockWatchy::drawTime()
{
  float batteryRange = clamp((getBatteryVoltage() - VOLTAGE_MIN) / VOLTAGE_RANGE);
  float batteryRangeSmooth = smoothstep(batteryRange);

  int hour = currentTime.Hour;
  int minute = currentTime.Minute;

  Vector3<float> minuteLight = Vector3<float>::rotateVector(Vector3<float>::lerp(MINUTE_LIGHT_LOW, MINUTE_LIGHT, batteryRange), MINUTE_ANGLE * minute);
  minuteLight.normalize();

  Vector3<float> coneBaseMinute1 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 1);
  coneBaseMinute1.z = 0.0f;
  coneBaseMinute1.normalize();
  coneBaseMinute1.scale(CONE_RADIUS);
  coneBaseMinute1 += CONE_CENTER;

  Vector3<float> coneBaseMinute2 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 3);
  coneBaseMinute2.z = 0.0f;
  coneBaseMinute2.normalize();
  coneBaseMinute2.scale(CONE_RADIUS);
  coneBaseMinute2 += CONE_CENTER;

  Vector3<float> coneTipMinute = CONE_TOP + minuteLight * (CONE_HEIGHT / Vector3<float>::dotProduct(NORMAL_DIR, minuteLight));

  Vector3<float> minuteHalfView = minuteLight + VIEW_DIR;
  minuteHalfView.normalize();

  Vector3<float> hourLight = Vector3<float>::rotateVector(Vector3<float>::lerp(HOUR_LIGHT_LOW, HOUR_LIGHT, batteryRange), HOUR_ANGLE * ((hour % 12) + minute/60.0) );
  hourLight.normalize();

  Vector3<float> coneBaseHour1 = Vector3<float>::rotateVectorByRightAngle(hourLight, 1);
  coneBaseHour1.z = 0.0f;
  coneBaseHour1.normalize();
  coneBaseHour1.scale(CONE_RADIUS);
  coneBaseHour1 += CONE_CENTER;

  Vector3<float> coneBaseHour2 = Vector3<float>::rotateVectorByRightAngle(hourLight, 3);
  coneBaseHour2.z = 0.0f;
  coneBaseHour2.normalize();
  coneBaseHour2.scale(CONE_RADIUS);
  coneBaseHour2 += CONE_CENTER;

  Vector3<float> coneTipHour = CONE_TOP + hourLight * (CONE_HEIGHT / Vector3<float>::dotProduct(NORMAL_DIR, hourLight));
  
  Vector3<float> hourHalfView = hourLight + VIEW_DIR;
  hourHalfView.normalize();
  
  display.startWrite();

  for (int y = 2; y < 198; y++)
  {
    for (int x = 2; x < 198; x++)
    {
      Vector3<float> point = {x,y,0.0f};
      Vector3<float> pointFromCenter = point - CONE_CENTER;
      float pointFromCenterSqrMagnitude = pointFromCenter.sqrMagnitude();

      if (pointFromCenterSqrMagnitude > WATCH_RADIUS_SQR)
        continue;

      int pixelIndex = x + 200 * y;
      int index = pixelIndex * 3;

      Vector3<float> normal = {Normal[index], Normal[index + 1], Normal[index + 2]};
      normal *= NORMAL_SCALE;
      //normal.normalize();

      float intensity = 0.0f;
      float specularIntensity = 0.0f;
      char gloss = Gloss[pixelIndex];
      float diffuse = Diffuse[pixelIndex];

      if (pointFromCenterSqrMagnitude > WATCH_RIM_RADIUS_SQR)
      {
        Vector3<float> pointFromCenterNormalized = pointFromCenter;
        pointFromCenterNormalized.normalize();
        float dot = Vector3<float>::dotProduct(UP, pointFromCenterNormalized) * 0.5f + 0.5f;

        if (batteryRangeSmooth > dot)
        {
          diffuse = 0.0f;
          gloss = 40;
        }
      }

      BlinnPhong(point, pointFromCenterSqrMagnitude, normal, minuteLight, minuteHalfView, gloss, coneTipMinute, coneBaseMinute1, coneBaseMinute2, intensity, specularIntensity);
      BlinnPhong(point, pointFromCenterSqrMagnitude, normal, hourLight, hourHalfView, gloss, coneTipHour, coneBaseHour1, coneBaseHour2, intensity, specularIntensity);

      bool white = (intensity * diffuse + specularIntensity) * 0.5f > BlueNoise200[pixelIndex];
      display.drawPixel(x, y, white ? GxEPD_WHITE : GxEPD_BLACK);
    }
  }
  
  display.endWrite();
}
