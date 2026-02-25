#include "../../watchy/Watchy.h"

#include <cassert>

#include "../../sdk/UiSDK.h"

#include "src/Images/WatchfaceBitmaps.h"
#include "src/Fonts/northrup_serif8pt7b.h"
#include "src/Screens/rle.h"

namespace {

int getPixel(int16_t x, int16_t y, const uint8_t *bitmap) {
  const int16_t imageWidth = pgm_read_byte(bitmap);
  const int16_t byteWidth = (imageWidth + 7) / 8;
  return (pgm_read_byte(bitmap + 2 + y * byteWidth + x / 8) & (128 >> (x & 7)));
}

void drawBitmapRotate(Watchy &watchy, int xx, int yy, int yOffset, const uint8_t *bitmap,
                      const uint8_t *borderBitmap, unsigned int fAngle, uint16_t color,
                      uint16_t borderColor) {
  const int iWidth = pgm_read_byte(bitmap);
  const int iHeight = pgm_read_byte(bitmap + 1);
  const int hX = iWidth / 2;
  const int hY = iHeight - yOffset;
  const float angle = fAngle * PI / 180.0;

  const int startX = -hX;
  const int endX = startX + iWidth;
  const int startY = -hY;
  const int endY = startY + iHeight;

  assert(hX <= hY);
  const int startAreaX = xx - hY;
  const int startAreaY = yy - hY;
  const int endAreaX = 200 - startAreaX;
  const int endAreaY = 200 - startAreaY;

  for (int x = startAreaX; x < endAreaX; x++) {
    yield();
    for (int y = startAreaY; y < endAreaY; y++) {
      const int ux = (x - xx) * cos(-angle) - (y - yy) * sin(-angle);
      const int uy = (x - xx) * sin(-angle) + (y - yy) * cos(-angle);

      if (ux >= startX && ux < endX && uy >= startY && uy < endY) {
        if (!getPixel(ux + hX, uy + hY, bitmap)) {
          UiSDK::drawPixel(watchy.display, x, y, color);
        } else if (!getPixel(ux + hX, uy + hY, borderBitmap)) {
          UiSDK::drawPixel(watchy.display, x, y, borderColor);
        }
      }
    }
  }
}

void drawDate(Watchy &watchy, uint16_t fgColor, uint16_t bgColor, const String &dayOfWeek,
              const String &date, int hourAngle, int minuteAngle) {
  int xAnchor = 100;
  int yAnchor = 140;
  bool topShade = false;

  if ((hourAngle < 151 || hourAngle > 213) && (minuteAngle < 146 || minuteAngle > 217)) {
    // center position
    xAnchor = 100;
    yAnchor = 140;
  } else if ((hourAngle < 52 || hourAngle > 144) && (minuteAngle < 57 || minuteAngle > 134)) {
    // right position
    xAnchor = 147;
    yAnchor = 91;
    topShade = true;
  } else if ((hourAngle < 216 || hourAngle > 300) && (minuteAngle < 224 || minuteAngle > 294)) {
    // left position
    xAnchor = 52;
    yAnchor = 91;
    topShade = true;
  }

  auto font = northrup_serif8pt7b;
  int16_t _x, _y;
  uint16_t w, h;
  UiSDK::setTextColor(watchy.display, fgColor);
  UiSDK::setTextSize(watchy.display, 0);
  UiSDK::setFont(watchy.display, font);

  UiSDK::getTextBounds(watchy.display, dayOfWeek, 0, 0, &_x, &_y, &w, &h);
  const int ys = yAnchor + h - 3;
  if (topShade) {
    UiSDK::setCursor(watchy.display, xAnchor - w / 2, ys + 1);
    UiSDK::setTextColor(watchy.display, bgColor);
    UiSDK::println(watchy.display, dayOfWeek);
    UiSDK::setTextColor(watchy.display, fgColor);
  }
  UiSDK::setCursor(watchy.display, xAnchor - w / 2, ys);
  UiSDK::println(watchy.display, dayOfWeek);

  UiSDK::getTextBounds(watchy.display, date, 0, 0, &_x, &_y, &w, &h);
  UiSDK::setCursor(watchy.display, xAnchor - w / 2, ys + font->yAdvance - 3);
  UiSDK::println(watchy.display, date);
}

} // namespace

void showWatchFace_Chronometer(Watchy &watchy) {
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::initScreen(watchy.display);
  drawRLEBitmap(watchy.display, 0, 0, &rle_background, fgColor);

  float batteryNorm = (watchy.getBatteryVoltage() - 3.3f) / 0.9f;
  if (batteryNorm < 0.0f) {
    batteryNorm = 0.0f;
  } else if (batteryNorm > 1.0f) {
    batteryNorm = 1.0f;
  }
  UiSDK::drawFastHLine(watchy.display, 63, 54, (int)(batteryNorm * 74.0f), fgColor);

  const int hourAngle = ((watchy.currentTime.Hour % 12) * 60 + watchy.currentTime.Minute) * 360 / 720;
  const int minuteAngle = watchy.currentTime.Minute * 6;

  drawDate(watchy, fgColor, bgColor,
           dayStr(watchy.currentTime.Wday),
           String(watchy.currentTime.Day) + ". " + String(monthStr(watchy.currentTime.Month)),
           hourAngle, minuteAngle);

  drawBitmapRotate(watchy, 100, 100, 6, fat_bitmap_hour_hand, fat_bitmap_hour_hand_border, hourAngle, fgColor,
                   bgColor);
  drawBitmapRotate(watchy, 100, 100, 8, fat_bitmap_minute_hand, fat_bitmap_minute_hand_border, minuteAngle,
                   fgColor, bgColor);
}
