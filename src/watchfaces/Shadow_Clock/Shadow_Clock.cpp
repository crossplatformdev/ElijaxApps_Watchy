#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "include/BlueNoise200.h"
#include "include/Diffuse.h"
#include "include/Gloss.h"
#include "include/GlossPow.h"
#include "include/Normal.h"
#include "src/Vector3.h"

namespace {

Watchy *g_watchy = nullptr;

const float kVoltageMin = 3.5f;
const float kVoltageMax = 4.2f;
const float kVoltageRange = kVoltageMax - kVoltageMin;

const float kMinuteAngle = 360.0f / 60.0f;
const float kHourAngle = 360.0f / 12.0f;

const float kConeRadius = 10.0f;
const float kConeRadiusSqr = kConeRadius * kConeRadius;
const float kConeHeight = 50.0f;

const float kWatchRadius = 98.0f;
const float kWatchRadiusSqr = kWatchRadius * kWatchRadius;

const float kWatchRimRadius = 93.0f;
const float kWatchRimRadiusSqr = kWatchRimRadius * kWatchRimRadius;

const float kNormalScale = 2.0f / 255.0f;

const Vector3<float> kUp = {0.0f, -1.0f, 0.0f};

const Vector3<float> kConeCenter = {99.5f, 99.5f, 0.0f};
const Vector3<float> kNormalDir = {0.0f, 0.0f, 1.0f};
const Vector3<float> kConeTop = kConeCenter + kNormalDir * kConeHeight;
const Vector3<float> kViewDir = {0.0f, 0.0f, 1.0f};

const Vector3<float> kMinuteLight = {0.0f, -0.75f, 0.6f};
const Vector3<float> kMinuteLightLow = {0.0f, -1.2f, 0.6f};

const Vector3<float> kHourLight = {0.0f, -0.5f, 0.6f};
const Vector3<float> kHourLightLow = {0.0f, -0.75f, 0.6f};

float clampFloat(float value) {
  if (value > 1.0f) {
    return 1.0f;
  }
  if (value < 0.0f) {
    return 0.0f;
  }
  return value;
}

float sign(const Vector3<float> &p1, const Vector3<float> &p2, const Vector3<float> &p3) {
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool pointInTriangle(const Vector3<float> &pt, const Vector3<float> &v1, const Vector3<float> &v2, const Vector3<float> &v3) {
  float d1 = sign(pt, v1, v2);
  float d2 = sign(pt, v2, v3);
  float d3 = sign(pt, v3, v1);

  bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  if (!hasNeg) {
    return true;
  }

  bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);
  return !hasPos;
}

float smoothstep(float x) {
  return x * x * (3.0f - 2.0f * x);
}

void blinnPhong(const Vector3<float> &point,
                const float &pointFromCenterSqrMagnitude,
                const Vector3<float> &normal,
                const Vector3<float> &lightDir,
                const Vector3<float> &halfView,
                const char &gloss,
                const Vector3<float> &coneBase1,
                const Vector3<float> &coneBase2,
                const Vector3<float> &coneBase3,
                float &intensity,
                float &specularIntensity) {
  if (pointFromCenterSqrMagnitude > kConeRadiusSqr && pointInTriangle(point, coneBase1, coneBase2, coneBase3)) {
    return;
  }

  float nDotL = Vector3<float>::dotProduct(normal, lightDir);
  intensity += smoothstep(clampFloat(nDotL));

  float nDotH = Vector3<float>::dotProduct(normal, halfView);
  specularIntensity += GlossPow[(int)(clampFloat(nDotH) * 255) + 256 * gloss];
}

void drawTime() {
  auto &display = g_watchy->display;

  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  float batteryRange = clampFloat((g_watchy->getBatteryVoltage() - kVoltageMin) / kVoltageRange);
  float batteryRangeSmooth = smoothstep(batteryRange);

  int hour = g_watchy->currentTime.Hour;
  int minute = g_watchy->currentTime.Minute;

  Vector3<float> minuteLight =
      Vector3<float>::rotateVector(Vector3<float>::lerp(kMinuteLightLow, kMinuteLight, batteryRange), kMinuteAngle * minute);
  minuteLight.normalize();

  Vector3<float> coneBaseMinute1 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 1);
  coneBaseMinute1.z = 0.0f;
  coneBaseMinute1.normalize();
  coneBaseMinute1.scale(kConeRadius);
  coneBaseMinute1 += kConeCenter;

  Vector3<float> coneBaseMinute2 = Vector3<float>::rotateVectorByRightAngle(minuteLight, 3);
  coneBaseMinute2.z = 0.0f;
  coneBaseMinute2.normalize();
  coneBaseMinute2.scale(kConeRadius);
  coneBaseMinute2 += kConeCenter;

  Vector3<float> coneTipMinute = kConeTop + minuteLight * (kConeHeight / Vector3<float>::dotProduct(kNormalDir, minuteLight));

  Vector3<float> minuteHalfView = minuteLight + kViewDir;
  minuteHalfView.normalize();

  Vector3<float> hourLight = Vector3<float>::rotateVector(Vector3<float>::lerp(kHourLightLow, kHourLight, batteryRange),
                                                          kHourAngle * ((hour % 12) + minute / 60.0f));
  hourLight.normalize();

  Vector3<float> coneBaseHour1 = Vector3<float>::rotateVectorByRightAngle(hourLight, 1);
  coneBaseHour1.z = 0.0f;
  coneBaseHour1.normalize();
  coneBaseHour1.scale(kConeRadius);
  coneBaseHour1 += kConeCenter;

  Vector3<float> coneBaseHour2 = Vector3<float>::rotateVectorByRightAngle(hourLight, 3);
  coneBaseHour2.z = 0.0f;
  coneBaseHour2.normalize();
  coneBaseHour2.scale(kConeRadius);
  coneBaseHour2 += kConeCenter;

  Vector3<float> coneTipHour = kConeTop + hourLight * (kConeHeight / Vector3<float>::dotProduct(kNormalDir, hourLight));

  Vector3<float> hourHalfView = hourLight + kViewDir;
  hourHalfView.normalize();

  UiSDK::startWrite(display);

  for (int y = 2; y < 198; y++) {
    for (int x = 2; x < 198; x++) {
      Vector3<float> point = {static_cast<float>(x), static_cast<float>(y), 0.0f};
      Vector3<float> pointFromCenter = point - kConeCenter;
      float pointFromCenterSqrMagnitude = pointFromCenter.sqrMagnitude();

      if (pointFromCenterSqrMagnitude > kWatchRadiusSqr) {
        continue;
      }

      int pixelIndex = x + 200 * y;
      int index = pixelIndex * 3;

      Vector3<float> normal = {static_cast<float>(Normal[index]), static_cast<float>(Normal[index + 1]),
                               static_cast<float>(Normal[index + 2])};
      normal *= kNormalScale;

      float intensity = 0.0f;
      float specularIntensity = 0.0f;
      char gloss = Gloss[pixelIndex];
      float diffuse = Diffuse[pixelIndex];

      if (pointFromCenterSqrMagnitude > kWatchRimRadiusSqr) {
        Vector3<float> pointFromCenterNormalized = pointFromCenter;
        pointFromCenterNormalized.normalize();
        float dot = Vector3<float>::dotProduct(kUp, pointFromCenterNormalized) * 0.5f + 0.5f;

        if (batteryRangeSmooth > dot) {
          diffuse = 0.0f;
          gloss = 40;
        }
      }

      blinnPhong(point, pointFromCenterSqrMagnitude, normal, minuteLight, minuteHalfView, gloss, coneTipMinute,
                 coneBaseMinute1, coneBaseMinute2, intensity, specularIntensity);
      blinnPhong(point, pointFromCenterSqrMagnitude, normal, hourLight, hourHalfView, gloss, coneTipHour,
                 coneBaseHour1, coneBaseHour2, intensity, specularIntensity);

      bool white = (intensity * diffuse + specularIntensity) * 0.5f > BlueNoise200[pixelIndex];
      UiSDK::drawPixel(display, x, y, white ? bgColor : fgColor);
    }
  }

  UiSDK::endWrite(display);
}

} // namespace

void showWatchFace_ShadowClock(Watchy &watchy) {
  g_watchy = &watchy;
  UiSDK::initScreen(watchy.display);
  drawTime();
}
