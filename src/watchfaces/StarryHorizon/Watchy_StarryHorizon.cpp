#include "WatchyUi.h"
#include "Watchy_StarryHorizon.h"
#include <Fonts/FreeSansBold9pt7b.h>
#include <math.h>
#include "MadeSunflower39pt7b.h"

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

void WatchyStarryHorizon::drawWatchFace(Watchy &watch) {
  Watchy::display.fillCircle(100, horizonY + planetRadius, planetRadius,
                     WatchyUi::Theme::foreground());
  drawGrid(watch);
  drawStars(watch);
  drawTime(watch);
  drawDate(watch);
}

void WatchyStarryHorizon::drawGrid(Watchy &) {
  int previousY = horizonY;
  for (int index = 0; index < 40; index++) {
    int y = previousY + fabs(sin(static_cast<double>(index) / 10.0) * 10.0);
    if (y <= DISPLAY_HEIGHT) {
      Watchy::display.drawFastHLine(0, y, DISPLAY_WIDTH,
                WatchyUi::Theme::background());
    }
    previousY = y;
  }

  constexpr int vanishingPointY = horizonY - 25;
  for (int x = -230; x < 430; x += 20) {
    Watchy::display.drawLine(x, DISPLAY_HEIGHT, DISPLAY_WIDTH / 2,
                     vanishingPointY, WatchyUi::Theme::background());
  }
}

void WatchyStarryHorizon::drawStars(Watchy &watch) {
  double angle = (twoPi / 60.0) * watch.currentTime.Minute;
  constexpr int starCount = sizeof(STARS) / sizeof(STARS[0]);
  for (int index = 0; index < starCount; index++) {
    Point rotated = rotatePoint(STARS[index].x, STARS[index].y, 100, 100,
                                angle);
    if (rotated.x < 0 || rotated.y < 0 || rotated.x >= DISPLAY_WIDTH ||
        rotated.y > horizonY) {
      continue;
    }
    if (STARS[index].r == 0) {
      Watchy::display.drawPixel(rotated.x, rotated.y,
                        WatchyUi::Theme::foreground());
    } else {
      Watchy::display.fillCircle(rotated.x, rotated.y, STARS[index].r,
                         WatchyUi::Theme::foreground());
    }
  }
}

void WatchyStarryHorizon::drawTime(Watchy &watch) {
  char timeText[8];
  snprintf(timeText, sizeof(timeText), "%d:%02d", watch.currentTime.Hour,
           watch.currentTime.Minute);
  Watchy::display.setFont(&MADE_Sunflower_PERSONAL_USE39pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setTextWrap(false);
  drawCenteredString(watch, timeText, 100, 115, false);
}

void WatchyStarryHorizon::drawDate(Watchy &watch) {
  String dateText = String(dayShortStr(watch.currentTime.Wday)) + " " +
                    monthShortStr(watch.currentTime.Month) + " " + watch.currentTime.Day;
  Watchy::display.setFont(&FreeSansBold9pt7b);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  Watchy::display.setTextWrap(false);
  drawCenteredString(watch, dateText, 100, 140, true);
}

void WatchyStarryHorizon::drawCenteredString(Watchy &, const String &text, int x, int y,
                                              bool drawBackground) {
  int16_t boundsX;
  int16_t boundsY;
  uint16_t width;
  uint16_t height;
  Watchy::display.getTextBounds(text, x, y, &boundsX, &boundsY, &width, &height);
  Watchy::display.setCursor(x - width / 2, y);
  if (drawBackground) {
    constexpr int horizontalPadding = 10;
    constexpr int verticalPadding = 3;
    Watchy::display.fillRect(x - (width / 2 + horizontalPadding),
                     y - (height + verticalPadding),
                     width + horizontalPadding * 2,
                     height + verticalPadding * 2,
                     WatchyUi::Theme::background());
  }
  Watchy::display.print(text);
}
