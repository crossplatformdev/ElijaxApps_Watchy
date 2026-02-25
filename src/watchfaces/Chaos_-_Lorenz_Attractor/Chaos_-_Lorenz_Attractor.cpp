#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <math.h>

#ifndef LORENZ_POINTS_PER_UPDATE
#define LORENZ_POINTS_PER_UPDATE 5
#endif
#ifndef LORENZ_MAX_POINTS
#define LORENZ_MAX_POINTS 500
#endif
#ifndef LORENZ_ROTATION_SPEED
#define LORENZ_ROTATION_SPEED 0.0523598776f
#endif
#ifndef TIMEZONE_OFFSET_HOURS
#define TIMEZONE_OFFSET_HOURS 1
#endif
#ifndef LATITUDE
#define LATITUDE 52.3676f
#endif
#ifndef LONGITUDE
#define LONGITUDE 4.9041f
#endif

namespace {

Watchy *g_watchy = nullptr;
uint16_t g_fgColor = 0;
uint16_t g_bgColor = 0;

float g_pos[3] = {1.0f, 1.0f, 1.0f};
float g_trajectory[LORENZ_MAX_POINTS][3];
int g_pointCount = 0;
int g_trajectoryIndex = 0;
float g_rotationAngle = 0.0f;

constexpr float kLorenzSigma = 10.0f;
constexpr float kLorenzRho = 28.0f;
constexpr float kLorenzBeta = 8.0f / 3.0f;
constexpr float kDeltaTime = 0.05f;

void lorenzDerivative(float p[3], float result[3]) {
  result[0] = kLorenzSigma * (p[1] - p[0]);
  result[1] = p[0] * (kLorenzRho - p[2]) - p[1];
  result[2] = p[0] * p[1] - kLorenzBeta * p[2];
}

void updateLorenz() {
  float k1[3], k2[3], k3[3], k4[3];
  float temp[3];

  lorenzDerivative(g_pos, k1);

  for (int i = 0; i < 3; i++) {
    temp[i] = g_pos[i] + k1[i] * kDeltaTime / 2.0f;
  }
  lorenzDerivative(temp, k2);

  for (int i = 0; i < 3; i++) {
    temp[i] = g_pos[i] + k2[i] * kDeltaTime / 2.0f;
  }
  lorenzDerivative(temp, k3);

  for (int i = 0; i < 3; i++) {
    temp[i] = g_pos[i] + k3[i] * kDeltaTime;
  }
  lorenzDerivative(temp, k4);

  for (int i = 0; i < 3; i++) {
    g_pos[i] += (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]) * kDeltaTime / 6.0f;
  }
}

void addTrajectoryPoint() {
  if (g_pointCount < LORENZ_MAX_POINTS) {
    g_trajectory[g_trajectoryIndex][0] = g_pos[0];
    g_trajectory[g_trajectoryIndex][1] = g_pos[1];
    g_trajectory[g_trajectoryIndex][2] = g_pos[2];
    g_trajectoryIndex++;
    g_pointCount++;
  } else {
    g_trajectory[g_trajectoryIndex][0] = g_pos[0];
    g_trajectory[g_trajectoryIndex][1] = g_pos[1];
    g_trajectory[g_trajectoryIndex][2] = g_pos[2];
    g_trajectoryIndex = (g_trajectoryIndex + 1) % LORENZ_MAX_POINTS;
    g_pointCount = LORENZ_MAX_POINTS;
  }
}

void drawLine(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    UiSDK::drawPixel(g_watchy->display, x0, y0, g_fgColor);

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void project3DTo2D(float point3D[3], float &x2D, float &y2D) {
  float cosRot = cos(g_rotationAngle);
  float sinRot = sin(g_rotationAngle);

  float xRot = point3D[0] * cosRot - point3D[2] * sinRot;
  float yRot = point3D[1];

  x2D = xRot;
  y2D = yRot;
}

void drawTrajectory() {
  if (g_pointCount == 0) {
    return;
  }

  int numPoints = (g_pointCount < LORENZ_MAX_POINTS) ? g_pointCount : LORENZ_MAX_POINTS;

  float screenX[LORENZ_MAX_POINTS];
  float screenY[LORENZ_MAX_POINTS];
  float minX = 999.0f;
  float maxX = -999.0f;
  float minY = 999.0f;
  float maxY = -999.0f;

  for (int i = 0; i < numPoints; i++) {
    int idx = (g_pointCount < LORENZ_MAX_POINTS) ? i : (g_trajectoryIndex + i) % LORENZ_MAX_POINTS;
    project3DTo2D(g_trajectory[idx], screenX[i], screenY[i]);

    if (screenX[i] < minX) {
      minX = screenX[i];
    }
    if (screenX[i] > maxX) {
      maxX = screenX[i];
    }
    if (screenY[i] < minY) {
      minY = screenY[i];
    }
    if (screenY[i] > maxY) {
      maxY = screenY[i];
    }
  }

  float rangeX = maxX - minX;
  float rangeY = maxY - minY;

  if (rangeX < 0.1f) {
    rangeX = 0.1f;
  }
  if (rangeY < 0.1f) {
    rangeY = 0.1f;
  }

  float scaleX = 120.0f / rangeX;
  float scaleY = 120.0f / rangeY;
  float scale = (scaleX < scaleY) ? scaleX : scaleY;

  float centerX = (minX + maxX) / 2.0f;
  float centerY = (minY + maxY) / 2.0f;

  for (int i = 0; i < numPoints; i++) {
    screenX[i] = (screenX[i] - centerX) * scale + 100.0f;
    screenY[i] = (screenY[i] - centerY) * scale + 100.0f;

    if (screenX[i] < 5.0f) {
      screenX[i] = 5.0f;
    }
    if (screenX[i] > 195.0f) {
      screenX[i] = 195.0f;
    }
    if (screenY[i] < 5.0f) {
      screenY[i] = 5.0f;
    }
    if (screenY[i] > 195.0f) {
      screenY[i] = 195.0f;
    }
  }

  for (int i = 0; i < numPoints; i++) {
    int x = static_cast<int>(screenX[i]);
    int y = static_cast<int>(screenY[i]);

    if (x >= 0 && x < 200 && y >= 0 && y < 200) {
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
          if (x + dx >= 0 && x + dx < 200 && y + dy >= 0 && y + dy < 200) {
            UiSDK::drawPixel(g_watchy->display, x + dx, y + dy, g_fgColor);
          }
        }
      }
    }

    if (i < numPoints - 1) {
      int nextI = i + 1;
      drawLine(static_cast<int>(screenX[i]), static_cast<int>(screenY[i]),
               static_cast<int>(screenX[nextI]), static_cast<int>(screenY[nextI]));
    }
  }

  if (numPoints > 0) {
    int lastIdx = (g_pointCount < LORENZ_MAX_POINTS) ? numPoints - 1
                                                     : (g_trajectoryIndex - 1 + LORENZ_MAX_POINTS) % LORENZ_MAX_POINTS;
    int currentX = static_cast<int>(screenX[lastIdx]);
    int currentY = static_cast<int>(screenY[lastIdx]);

    if (currentX >= 0 && currentX < 200 && currentY >= 0 && currentY < 200) {
      for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
          if (currentX + dx >= 0 && currentX + dx < 200 && currentY + dy >= 0 && currentY + dy < 200) {
            UiSDK::drawPixel(g_watchy->display, currentX + dx, currentY + dy, g_fgColor);
          }
        }
      }
    }
  }
}

void drawLorenzAttractor() {
  for (int i = 0; i < LORENZ_POINTS_PER_UPDATE; i++) {
    updateLorenz();
    addTrajectoryPoint();
  }

  g_rotationAngle += LORENZ_ROTATION_SPEED;
  if (g_rotationAngle > 2 * PI) {
    g_rotationAngle -= 2 * PI;
  }

  drawTrajectory();
}

float calculateMoonPhase() {
  const float moonCycleDays = 29.53058867f;

  int year = g_watchy->currentTime.Year + 1970;
  int month = g_watchy->currentTime.Month;
  int day = g_watchy->currentTime.Day;
  int hour = g_watchy->currentTime.Hour;
  int minute = g_watchy->currentTime.Minute;
  int second = g_watchy->currentTime.Second;

  long daysSince2000 = 0;
  for (int y = 2000; y < year; y++) {
    daysSince2000 += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
  }

  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
    daysInMonth[1] = 29;
  }
  for (int m = 1; m < month; m++) {
    daysSince2000 += daysInMonth[m - 1];
  }

  daysSince2000 += day - 1;

  long utcSeconds = daysSince2000 * 86400L;
  utcSeconds += hour * 3600L;
  utcSeconds += minute * 60L;
  utcSeconds += second;

  long timezoneOffsetSeconds = TIMEZONE_OFFSET_HOURS * 3600L;
  utcSeconds -= timezoneOffsetSeconds;

  const float refNewMoonDays = 5.0f + (18.0f / 24.0f) + (14.0f / 1440.0f);
  const float refNewMoonSeconds = refNewMoonDays * 86400.0f;

  float secondsSinceNewMoon = static_cast<float>(utcSeconds) - refNewMoonSeconds;
  float daysSinceNewMoon = secondsSinceNewMoon / 86400.0f;

  float daysInCycle = fmod(daysSinceNewMoon, moonCycleDays);
  if (daysInCycle < 0) {
    daysInCycle += moonCycleDays;
  }

  return daysInCycle / moonCycleDays;
}

void drawMoonPhase() {
  float phase = calculateMoonPhase();

  int moonRadius = 10;
  int moonX = 200 - moonRadius - 8;
  int moonY = moonRadius + 2;

  bool waxing = (phase < 0.5f);
  float illumination = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;

  for (int x = -moonRadius; x <= moonRadius; x++) {
    for (int y = -moonRadius; y <= moonRadius; y++) {
      float dist = sqrt(static_cast<float>(x * x + y * y));
      int px = moonX + x;
      int py = moonY + y;

      if (px < 0 || px >= 200 || py < 0 || py >= 200) {
        continue;
      }

      if (dist > moonRadius - 0.5f && dist <= moonRadius + 0.5f) {
        UiSDK::drawPixel(g_watchy->display, px, py, g_fgColor);
      } else if (dist < moonRadius - 1) {
        bool shouldFill = false;

        if (illumination > 0.95f) {
          shouldFill = true;
        } else if (illumination >= 0.05f) {
          float xPos = x / static_cast<float>(moonRadius);

          if (waxing) {
            float threshold = -1.0f + 2.0f * illumination;
            shouldFill = (xPos >= threshold);
          } else {
            float threshold = 1.0f - 2.0f * illumination;
            shouldFill = (xPos <= threshold);
          }
        }

        if (shouldFill) {
          UiSDK::drawPixel(g_watchy->display, px, py, g_fgColor);
        }
      }
    }
  }
}

int calculateDayOfYear() {
  int year = g_watchy->currentTime.Year + 1970;
  int month = g_watchy->currentTime.Month;
  int day = g_watchy->currentTime.Day;

  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
    daysInMonth[1] = 29;
  }

  int dayOfYear = day;
  for (int m = 1; m < month; m++) {
    dayOfYear += daysInMonth[m - 1];
  }

  return dayOfYear;
}

void calculateSunriseSunset(float &sunriseHour, float &sunsetHour) {
  float latRad = LATITUDE * PI / 180.0f;
  int dayOfYear = calculateDayOfYear();

  float declination = 23.45f * PI / 180.0f * sin(2.0f * PI * (284.0f + dayOfYear) / 365.0f);
  float solarElevation = -0.83f * PI / 180.0f;

  float cosHourAngle = (sin(solarElevation) - sin(latRad) * sin(declination)) /
                       (cos(latRad) * cos(declination));

  if (cosHourAngle > 1.0f) {
    sunriseHour = -1.0f;
    sunsetHour = -1.0f;
    return;
  }
  if (cosHourAngle < -1.0f) {
    sunriseHour = 0.0f;
    sunsetHour = 24.0f;
    return;
  }

  float hourAngle = acos(cosHourAngle);
  float B = 2.0f * PI * (dayOfYear - 81) / 365.0f;
  float equationOfTime = 9.87f * sin(2.0f * B) - 7.53f * cos(B) - 1.5f * sin(B);

  float longitudeTimeOffset = LONGITUDE / 15.0f;
  float timeCorrection = longitudeTimeOffset - TIMEZONE_OFFSET_HOURS + (equationOfTime / 60.0f);

  float solarNoon = 12.0f - timeCorrection;

  float sunriseSolar = solarNoon - (hourAngle * 12.0f / PI);
  float sunsetSolar = solarNoon + (hourAngle * 12.0f / PI);

  sunriseHour = sunriseSolar;
  sunsetHour = sunsetSolar;

  if (sunriseHour < 0.0f) {
    sunriseHour += 24.0f;
  }
  if (sunriseHour >= 24.0f) {
    sunriseHour -= 24.0f;
  }
  if (sunsetHour < 0.0f) {
    sunsetHour += 24.0f;
  }
  if (sunsetHour >= 24.0f) {
    sunsetHour -= 24.0f;
  }
}

void drawSunriseSunset() {
  float sunriseHour = 0.0f;
  float sunsetHour = 0.0f;
  calculateSunriseSunset(sunriseHour, sunsetHour);

  if (sunriseHour < 0 || sunsetHour < 0) {
    return;
  }

  int sunriseH = static_cast<int>(sunriseHour);
  int sunriseM = static_cast<int>((sunriseHour - sunriseH) * 60.0f);
  int sunsetH = static_cast<int>(sunsetHour);
  int sunsetM = static_cast<int>((sunsetHour - sunsetH) * 60.0f);

  String sunriseStr;
  if (sunriseH < 10) {
    sunriseStr += "0";
  }
  sunriseStr += String(sunriseH);
  if (sunriseM < 10) {
    sunriseStr += "0";
  }
  sunriseStr += String(sunriseM);

  String sunsetStr;
  if (sunsetH < 10) {
    sunsetStr += "0";
  }
  sunsetStr += String(sunsetH);
  if (sunsetM < 10) {
    sunsetStr += "0";
  }
  sunsetStr += String(sunsetM);

  String displayStr = sunriseStr + "/" + sunsetStr;

  UiSDK::setFont(g_watchy->display, &FreeMonoBold9pt7b);

  int16_t x1, y1;
  uint16_t w, h;
  UiSDK::getTextBounds(g_watchy->display, displayStr, 0, 0, &x1, &y1, &w, &h);

  int centerX = (200 - w) / 2;
  UiSDK::setCursor(g_watchy->display, centerX, 200 - 15 + 5);
  UiSDK::print(g_watchy->display, displayStr);
}

} // namespace

void showWatchFace_ChaosLorenzAttractor(Watchy &watchy) {
  g_watchy = &watchy;

  g_fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  g_bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  UiSDK::initScreen(watchy.display);
  UiSDK::fillScreen(watchy.display, g_bgColor);
  UiSDK::setTextColor(watchy.display, g_fgColor);
  UiSDK::setTextWrap(watchy.display, false);

  drawLorenzAttractor();

  UiSDK::setFont(watchy.display, &FreeMonoBold9pt7b);

  String textstring;
  if (watchy.currentTime.Hour < 10) {
    textstring = "0";
  }
  textstring += watchy.currentTime.Hour;
  textstring += ":";
  if (watchy.currentTime.Minute < 10) {
    textstring += "0";
  }
  textstring += watchy.currentTime.Minute;

  UiSDK::setCursor(watchy.display, 5, 19);
  UiSDK::print(watchy.display, textstring);

  int16_t x1, y1;
  uint16_t w, h;
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  int timeWidth = w;
  int timeEndX = 5 + timeWidth;

  drawMoonPhase();

  UiSDK::setFont(watchy.display, &FreeMonoBold9pt7b);
  textstring = dayShortStr(watchy.currentTime.Wday);
  textstring += " ";
  textstring += watchy.currentTime.Day;
  textstring += "/";
  textstring += watchy.currentTime.Month;

  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  int moonRadius = 10;
  int moonX = 200 - moonRadius - 8;
  int moonLeftEdge = moonX - moonRadius;
  int maxDateX = moonLeftEdge - 5;

  int dateStartX = timeEndX + 12;
  if (dateStartX + w > maxDateX) {
    dateStartX = maxDateX - w;
    if (dateStartX < timeEndX + 5) {
      dateStartX = timeEndX + 5;
    }
  }
  if (dateStartX + w > 200) {
    dateStartX = 200 - w;
  }

  UiSDK::setCursor(watchy.display, dateStartX, 19);
  UiSDK::print(watchy.display, textstring);

  float vbat = watchy.getBatteryVoltage();
  float batteryMin = 3.0f;
  float batteryMax = 4.2f;
  float batteryPercent = (vbat - batteryMin) / (batteryMax - batteryMin);
  if (batteryPercent < 0.0f) {
    batteryPercent = 0.0f;
  }
  if (batteryPercent > 1.0f) {
    batteryPercent = 1.0f;
  }

  int batteryX = 200 - 25;
  int batteryY = 200 - 15;
  int batteryWidth = 20;
  int batteryHeight = 10;

  UiSDK::drawRect(watchy.display, batteryX, batteryY, batteryWidth, batteryHeight, g_fgColor);
  UiSDK::drawRect(watchy.display, batteryX + batteryWidth, batteryY + 2, 2, 6, g_fgColor);

  int fillWidth = static_cast<int>((batteryWidth - 2) * batteryPercent);
  if (fillWidth > 0) {
    UiSDK::fillRect(watchy.display, batteryX + 1, batteryY + 1, fillWidth, batteryHeight - 2, g_fgColor);
  }

  textstring = String(sensor.getCounter());
  UiSDK::getTextBounds(watchy.display, textstring, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(watchy.display, 5, 200 - 15 + 5);
  UiSDK::print(watchy.display, textstring);

  drawSunriseSunset();
}
