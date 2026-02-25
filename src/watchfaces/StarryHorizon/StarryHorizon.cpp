// STARRY HORIZON for Watchy by SQFMI
// Copyright 2021 Dan Delany dan.delany@gmail.com
// Released under free MIT License : https://github.com/dandelany/watchy-faces/blob/main/LICENSE

#include "StarryHorizon.h"

#include <Fonts/FreeSansBold9pt7b.h>

#include "MadeSunflower39pt7b.h"
#include "stars.h"

#include "../../sdk/UiSDK.h"

#define STAR_COUNT 900

static const int horizonY = 150;
static const int planetR = 650;

struct xyPoint {
  int x;
  int y;
};

using WatchyDisplayT = decltype(Watchy::display);

static xyPoint rotatePointAround(int x, int y, int ox, int oy, double angle) {
  double qx = (double)ox + (cos(angle) * (double)(x - ox)) + (sin(angle) * (double)(y - oy));
  double qy = (double)oy + (-sin(angle) * (double)(x - ox)) + (cos(angle) * (double)(y - oy));
  xyPoint newPoint;
  newPoint.x = (int)qx;
  newPoint.y = (int)qy;
  return newPoint;
}

static void drawCenteredString(WatchyDisplayT &display, const String &str, int x, int y, bool drawBg, uint16_t bgColor) {
  int16_t x1, y1;
  uint16_t w, h;

  UiSDK::getTextBounds(display, str, x, y, &x1, &y1, &w, &h);
  UiSDK::setCursor(display, x - w / 2, y);
  if (drawBg) {
    int padY = 3;
    int padX = 10;
    UiSDK::fillRect(display, x - (w / 2 + padX), y - (h + padY), w + padX * 2, h + padY * 2, bgColor);
  }
  UiSDK::print(display, str);
}

static void drawGrid(WatchyDisplayT &display, uint16_t gridColor) {
  int prevY = horizonY;
  for (int i = 0; i < 40; i += 1) {
    int y = prevY + int(abs(sin(double(i) / 10) * 10));
    if (y <= 200) {
      UiSDK::drawFastHLine(display, 0, y, 200, gridColor);
    }
    prevY = y;
  }

  int vanishY = horizonY - 25;
  for (int x = -230; x < 430; x += 20) {
    UiSDK::drawLine(display, x, 200, 100, vanishY, gridColor);
  }
}

static void drawStars(WatchyDisplayT &display, tmElements_t currentTime, const Star stars[], uint16_t starColor) {
  int minute = (int)currentTime.Minute;
  double minuteAngle = ((2.0 * M_PI) / 60.0) * (double)minute;

  for (int starI = 0; starI < STAR_COUNT; starI++) {
    int starX = stars[starI].x;
    int starY = stars[starI].y;
    int starR = stars[starI].r;

    xyPoint rotated = rotatePointAround(starX, starY, 100, 100, minuteAngle);
    if (rotated.x < 0 || rotated.y < 0 || rotated.x > 200 || rotated.y > horizonY) {
      continue;
    }
    if (starR == 0) {
      UiSDK::drawPixel(display, rotated.x, rotated.y, starColor);
    } else {
      UiSDK::fillCircle(display, rotated.x, rotated.y, starR, starColor);
    }
  }
}

static void drawTime(WatchyDisplayT &display, tmElements_t currentTime, uint16_t fgColor, uint16_t bgColor) {
  UiSDK::setFont(display, &MADE_Sunflower_PERSONAL_USE39pt7b);
  UiSDK::setTextColor(display, fgColor);
  UiSDK::setTextWrap(display, false);
  char *timeStr;
  asprintf(&timeStr, "%d:%02d", currentTime.Hour, currentTime.Minute);
  drawCenteredString(display, timeStr, 100, 115, false, bgColor);
  free(timeStr);
}

static void drawDate(WatchyDisplayT &display, tmElements_t currentTime, uint16_t fgColor, uint16_t bgColor) {
  String monthStr = monthShortStr(currentTime.Month);
  String dayOfWeek = dayShortStr(currentTime.Wday);
  UiSDK::setFont(display, &FreeSansBold9pt7b);
  UiSDK::setTextColor(display, fgColor);
  UiSDK::setTextWrap(display, false);
  char *dateStr;
  asprintf(&dateStr, "%s %s %d", dayOfWeek.c_str(), monthStr.c_str(), currentTime.Day);
  drawCenteredString(display, dateStr, 100, 140, true, bgColor);
  free(dateStr);
}

void WatchyStarryHorizon::drawWatchFace() {
  UiSDK::initScreen(display);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::fillCircle(display, 100, horizonY + planetR, planetR, fgColor);
  drawGrid(display, bgColor);
  drawStars(display, currentTime, STARS, fgColor);
  drawTime(display, currentTime, fgColor, bgColor);
  drawDate(display, currentTime, fgColor, bgColor);
}
// Copyright 2021 Dan Delany dan.delany@gmail.com

// Released under free MIT License : https://github.com/dandelany/watchy-faces/blob/main/LICENSE



void showWatchFace_StarryHorizon(Watchy &watchy) {
  WatchyStarryHorizon face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}


