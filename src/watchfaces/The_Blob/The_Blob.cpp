#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "include/BlueNoise200.h"
#include "include/MatCapSource.h"
#include "include/images.h"
#include "src/Vector.h"
#include "src/VectorInt.h"

namespace {

Watchy *g_watchy = nullptr;
uint16_t g_fgColor = 0;
uint16_t g_bgColor = 0;

const float kVoltageMin = 3.4f;
const float kVoltageMax = 4.2f;
const float kVoltageWarning = 3.5f;
const float kVoltageRange = kVoltageMax - kVoltageMin;

const int kDensity = 2;
const int kVectorSize = 60 * kDensity;
const double kStepAngle = 360.0 / kVectorSize;

const int kStepMinute = kDensity;
const int kStepHour = kVectorSize / 12;

const Vector kCenter = {100, 100};
const int kRadius = 99;
const int kBlobRadius = 90;

const int kHourIndentDistance = 10;
const double kHourIndentStrength = 0.2;

const int kMinuteIndentDistance = 4;
const double kMinuteIndentStrength = 0.35;

const double kBatteryMin = 0.5;
const double kBatteryRange = 1.0 - kBatteryMin;
const double kBatteryWarning = kBatteryMin + ((kVoltageWarning - kVoltageMin) / kVoltageRange) * kBatteryRange;

const double kHourTick = 1 - 0.1;
const double kMinuteTick = 1 - 0.3;

static Vector s_edgeVectors[kVectorSize];
static Vector s_edgeNormal[kVectorSize];
static bool s_edgeInit = false;

void initEdgeVectors() {
  if (s_edgeInit) {
    return;
  }

  Vector up = {0.0, -1.0};
  for (int i = 0; i < kVectorSize; i++) {
    s_edgeNormal[i] = s_edgeVectors[i] = Vector::rotateVector(up, i * kStepAngle);
    s_edgeNormal[i].normalize();
    s_edgeVectors[i].normalize();
  }

  s_edgeInit = true;
}

double smoothstep(double x) {
  return x * x * (3.0 - 2.0 * x);
}

void indent(int distance, int centerIndex, double amount, double scale[]) {
  double divider = 1.0 / static_cast<double>(distance);

  for (int i = -distance + 1; i < distance; i++) {
    int index = (i + centerIndex + kVectorSize) % kVectorSize;
    double strength = amount * smoothstep(1.0 - abs(i * divider));
    scale[index] -= strength;
  }
}

void recalculateNormal(int start, int length, double scale[], Vector normals[]) {
  int startIndex = (start + kVectorSize) % kVectorSize;
  int prevIndex = (start - 1 + kVectorSize) % kVectorSize;

  Vector prevVector = s_edgeVectors[prevIndex] * scale[prevIndex];
  Vector startVector = s_edgeVectors[startIndex] * scale[startIndex];

  Vector prevNormal = prevVector - startVector;
  prevNormal.normalize();
  prevNormal = Vector::rotateVectorByRightAngle(prevNormal, 1);

  for (int i = 0; i < length + 1; i++) {
    int minuteToIndex = i + start;
    int index = (minuteToIndex + kVectorSize) % kVectorSize;
    int nextIndex = (minuteToIndex + 1 + kVectorSize) % kVectorSize;

    Vector v1 = s_edgeVectors[index] * scale[index];
    Vector v2 = s_edgeVectors[nextIndex] * scale[nextIndex];

    Vector nextNormal = v1 - v2;
    nextNormal.normalize();
    nextNormal = Vector::rotateVectorByRightAngle(nextNormal, 1);

    Vector normal = prevNormal + nextNormal;
    normal.normalize();

    prevNormal = nextNormal;
    normals[index] = normal;
  }
}

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                                            \
  {                                                                                                    \
    int16_t t = a;                                                                                     \
    a = b;                                                                                             \
    b = t;                                                                                             \
  }
#endif

#ifndef _swap_vector
#define _swap_vector(a, b)                                                                             \
  {                                                                                                    \
    Vector t = a;                                                                                      \
    a = b;                                                                                             \
    b = t;                                                                                             \
  }
#endif

#ifndef _swap_vector_int
#define _swap_vector_int(a, b)                                                                         \
  {                                                                                                    \
    VectorInt t = a;                                                                                   \
    a = b;                                                                                             \
    b = t;                                                                                             \
  }
#endif

void barycentric2(VectorInt p, VectorInt v0, VectorInt v1, VectorInt a, double invDen, double &u, double &v, double &w) {
  VectorInt v2 = p - a;
  v = (v2.x * v1.y - v1.x * v2.y) * invDen;
  w = (v0.x * v2.y - v2.x * v0.y) * invDen;
  u = 1.0 - v - w;
}

bool getColor(int16_t x, int16_t y, int16_t xUv, int16_t yUv, const uint8_t *bitmap, int16_t w, int16_t h) {
  return bitmap[yUv * w + xUv] > BlueNoise200[y * 200 + x];
}

void drawLine2(int x, int y, int w, VectorInt v0, Vector uv0, VectorInt a, Vector uv1, VectorInt b, Vector uv2, double invDen,
               const uint8_t *bitmap, int16_t bw, int16_t bh) {
  auto &display = g_watchy->display;

  for (int i = 0; i < w; i++) {
    double ua, va, wa;
    VectorInt pointA = {x + i, y};
    barycentric2(pointA, a, b, v0, invDen, ua, va, wa);

    Vector uv = uv0 * ua + uv1 * va + uv2 * wa;

    bool white = getColor(x + i, y, uv.x, uv.y, bitmap, bw, bh);
    UiSDK::drawPixel(display, x + i, y, white ? g_bgColor : g_fgColor);
  }
}

void fillTriangle(VectorInt v0, Vector uv0, VectorInt v1, Vector uv1, VectorInt v2, Vector uv2, const uint8_t bitmap[], int w, int h) {
  auto &display = g_watchy->display;
  int16_t a, b, y, last;
  Vector uvA, uvB;

  if (v0.y > v1.y) {
    _swap_vector_int(v0, v1);
    _swap_vector(uv0, uv1);
  }
  if (v1.y > v2.y) {
    _swap_vector_int(v2, v1);
    _swap_vector(uv2, uv1);
  }
  if (v0.y > v1.y) {
    _swap_vector_int(v0, v1);
    _swap_vector(uv0, uv1);
  }

  UiSDK::startWrite(display);
  if (v0.y == v2.y) {
    a = b = v0.x;
    uvA = uv0;
    uvB = uv0;

    if (v1.x < a) {
      a = v1.x;
      uvA = uv1;
    } else if (v1.x > b) {
      b = v1.x;
      uvB = uv1;
    }
    if (v2.x < a) {
      a = v2.x;
      uvA = uv2;
    } else if (v2.x > b) {
      b = v2.x;
      uvB = uv2;
    }

    for (int i = 0; i < b - a + 1; i++) {
      double lerpVal = i / (b - a + 2.0);
      Vector uv = (uvA * lerpVal) + (uvB * (1.0 - lerpVal));
      bool white = getColor(a + i, v0.y, uv.x, uv.y, bitmap, w, h);
      UiSDK::drawPixel(display, a + i, v0.y, white ? g_bgColor : g_fgColor);
    }

    UiSDK::endWrite(display);
    return;
  }

  int16_t dx01 = v1.x - v0.x;
  int16_t dy01 = v1.y - v0.y;
  int16_t dx02 = v2.x - v0.x;
  int16_t dy02 = v2.y - v0.y;
  int16_t dx12 = v2.x - v1.x;
  int16_t dy12 = v2.y - v1.y;
  int32_t sa = 0;
  int32_t sb = 0;

  if (v1.y == v2.y) {
    last = v1.y;
  } else {
    last = v1.y - 1;
  }

  VectorInt aa = v1 - v0;
  VectorInt bb = v2 - v0;
  double invDen = 1 / VectorInt::crossProduct(aa, bb);

  for (y = v0.y; y <= last; y++) {
    a = v0.x + sa / dy01;
    b = v0.x + sb / dy02;

    sa += dx01;
    sb += dx02;

    if (a > b) {
      _swap_int16_t(a, b);
    }

    drawLine2(a, y, b - a + 1, v0, uv0, aa, uv1, bb, uv2, invDen, bitmap, w, h);
  }

  sa = static_cast<int32_t>(dx12) * (y - v1.y);
  sb = static_cast<int32_t>(dx02) * (y - v0.y);
  for (; y <= v2.y; y++) {
    a = v1.x + sa / dy12;
    b = v0.x + sb / dy02;

    sa += dx12;
    sb += dx02;

    if (a > b) {
      _swap_int16_t(a, b);
    }

    drawLine2(a, y, b - a + 1, v0, uv0, aa, uv1, bb, uv2, invDen, bitmap, w, h);
  }

  UiSDK::endWrite(display);
}

double getBatteryFill() {
  float vbat = g_watchy->getBatteryVoltage();

  double batState = (vbat - kVoltageMin) / kVoltageRange;
  if (batState > 1.0) {
    batState = 1.0;
  }
  if (batState < 0.0) {
    batState = 0.0;
  }

  return batState;
}

} // namespace

void showWatchFace_Blob(Watchy &watchy) {
  g_watchy = &watchy;
  initEdgeVectors();

  UiSDK::initScreen(watchy.display);
  g_bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  g_fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::setTextColor(watchy.display, g_fgColor);

  double scale[kVectorSize];
  Vector normals[kVectorSize];

  for (int i = 0; i < kVectorSize; i++) {
    scale[i] = 1.0;
    normals[i] = s_edgeNormal[i];
  }

  int hour = watchy.currentTime.Hour;
  int minute = watchy.currentTime.Minute;

  int minuteIndex = minute * kStepMinute;
  int hourIndex = round(((double)(hour % 12) + minute / 60.0) * kStepHour);

  indent(kMinuteIndentDistance, minuteIndex, kMinuteIndentStrength, scale);
  indent(kHourIndentDistance, hourIndex, kHourIndentStrength, scale);

  int minuteStart = minuteIndex - kMinuteIndentDistance;
  int minuteEnd = minuteIndex + kMinuteIndentDistance;

  int hourStart = hourIndex - kHourIndentDistance;
  int hourEnd = hourIndex + kHourIndentDistance;

  if ((minuteStart <= hourEnd && minuteStart >= hourStart) || (hourStart <= minuteEnd && hourStart >= minuteStart)) {
    int start = min(minuteStart, hourStart);
    int end = min(minuteEnd, hourEnd);

    recalculateNormal(start, end - start, scale, normals);
  } else {
    recalculateNormal(minuteStart, minuteEnd - minuteStart, scale, normals);
    recalculateNormal(hourStart, hourEnd - hourStart, scale, normals);
  }

  double batteryFill = getBatteryFill();
  double batteryFillScale = kBatteryMin + kBatteryRange * batteryFill;

  for (int i = 0; i < kVectorSize; i += kStepMinute) {
    Vector v1 = s_edgeVectors[i] * (kRadius) + kCenter;
    Vector v2 = s_edgeVectors[i] * (kRadius * kMinuteTick) + kCenter;

    UiSDK::drawLine(watchy.display, v1.x, v1.y, v2.x, v2.y, g_fgColor);
  }

  for (int i = 0; i < kVectorSize; i += kStepHour) {
    Vector v1 = s_edgeVectors[i] * (kRadius) + kCenter;
    Vector v2 = s_edgeVectors[i] * (kRadius * kHourTick) + kCenter;

    UiSDK::drawLine(watchy.display, v1.x + 1, v1.y, v2.x + 1, v2.y, g_fgColor);
    UiSDK::drawLine(watchy.display, v1.x - 1, v1.y, v2.x - 1, v2.y, g_fgColor);
    UiSDK::drawLine(watchy.display, v1.x, v1.y + 1, v2.x, v2.y + 1, g_fgColor);
    UiSDK::drawLine(watchy.display, v1.x, v1.y - 1, v2.x, v2.y - 1, g_fgColor);
  }

  UiSDK::drawCircle(watchy.display, kCenter.x, kCenter.y, kBlobRadius * kBatteryWarning, g_fgColor);

  for (int i = 0; i < kVectorSize; i++) {
    int nextIndex = (i + 1) % kVectorSize;
    Vector v1 = s_edgeVectors[i] * (kBlobRadius * scale[i] * batteryFillScale) + kCenter;
    Vector uv1 = normals[i] * kRadius + kCenter;
    Vector v2 = s_edgeVectors[nextIndex] * (kBlobRadius * scale[nextIndex] * batteryFillScale) + kCenter;
    Vector uv2 = normals[nextIndex] * kRadius + kCenter;

    fillTriangle(kCenter, kCenter, v1, uv1, v2, uv2, MatCapSource, 200, 200);
    UiSDK::drawLine(watchy.display, v1.x, v1.y, v2.x, v2.y, g_fgColor);
  }
}
