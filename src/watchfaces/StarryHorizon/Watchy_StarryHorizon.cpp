#include "Watchy_StarryHorizon.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <math.h>
#include "fonts/MadeSunflower39pt7b.h"
#include "sdk/WatchyUi.h"
#include "stars.h"

namespace {

constexpr int horizonY = 150;
constexpr int planetRadius = 650;
constexpr double twoPi = 6.28318530717958647692;

struct Point {
  int x;
  int y;
};

Point rotatePoint(int x, int y, int originX, int originY, double angle) {
  Point rotated;
  rotated.x = originX + cos(angle) * (x - originX) +
              sin(angle) * (y - originY);
  rotated.y = originY - sin(angle) * (x - originX) +
              cos(angle) * (y - originY);
  return rotated;
}

} // namespace

void WatchyStarryHorizon::drawWatchFace() {
  display.fillCircle(100, horizonY + planetRadius, planetRadius,
                     WatchyUi::Theme::foreground());
  drawGrid();
  drawStars();
  drawTime();
  drawDate();
}

void WatchyStarryHorizon::drawGrid() {
  int previousY = horizonY;
  for (int index = 0; index < 40; index++) {
    int y = previousY + fabs(sin(static_cast<double>(index) / 10.0) * 10.0);
    if (y <= DISPLAY_HEIGHT) {
      display.drawFastHLine(0, y, DISPLAY_WIDTH,
                WatchyUi::Theme::background());
    }
    previousY = y;
  }

  constexpr int vanishingPointY = horizonY - 25;
  for (int x = -230; x < 430; x += 20) {
    display.drawLine(x, DISPLAY_HEIGHT, DISPLAY_WIDTH / 2,
                     vanishingPointY, WatchyUi::Theme::background());
  }
}

void WatchyStarryHorizon::drawStars() {
  double angle = (twoPi / 60.0) * currentTime.Minute;
  constexpr int starCount = sizeof(STARS) / sizeof(STARS[0]);
  for (int index = 0; index < starCount; index++) {
    Point rotated = rotatePoint(STARS[index].x, STARS[index].y, 100, 100,
                                angle);
    if (rotated.x < 0 || rotated.y < 0 || rotated.x >= DISPLAY_WIDTH ||
        rotated.y > horizonY) {
      continue;
    }
    if (STARS[index].r == 0) {
      display.drawPixel(rotated.x, rotated.y,
                        WatchyUi::Theme::foreground());
    } else {
      display.fillCircle(rotated.x, rotated.y, STARS[index].r,
                         WatchyUi::Theme::foreground());
    }
  }
}

void WatchyStarryHorizon::drawTime() {
  char timeText[6];
  snprintf(timeText, sizeof(timeText), "%d:%02d", currentTime.Hour,
           currentTime.Minute);
  display.setFont(&MADE_Sunflower_PERSONAL_USE39pt7b);
  display.setTextColor(WatchyUi::Theme::foreground());
  display.setTextWrap(false);
  drawCenteredString(timeText, 100, 115, false);
}

void WatchyStarryHorizon::drawDate() {
  String dateText = String(dayShortStr(currentTime.Wday)) + " " +
                    monthShortStr(currentTime.Month) + " " + currentTime.Day;
  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(WatchyUi::Theme::foreground());
  display.setTextWrap(false);
  drawCenteredString(dateText, 100, 140, true);
}

void WatchyStarryHorizon::drawCenteredString(const String &text, int x, int y,
                                              bool drawBackground) {
  int16_t boundsX;
  int16_t boundsY;
  uint16_t width;
  uint16_t height;
  display.getTextBounds(text, x, y, &boundsX, &boundsY, &width, &height);
  display.setCursor(x - width / 2, y);
  if (drawBackground) {
    constexpr int horizontalPadding = 10;
    constexpr int verticalPadding = 3;
    display.fillRect(x - (width / 2 + horizontalPadding),
                     y - (height + verticalPadding),
                     width + horizontalPadding * 2,
                     height + verticalPadding * 2,
                     WatchyUi::Theme::background());
  }
  display.print(text);
}