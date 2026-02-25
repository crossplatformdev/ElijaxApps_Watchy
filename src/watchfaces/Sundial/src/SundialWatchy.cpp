#include "SundialWatchy.h"
#include "sdk/UiSDK.h"

const float VOLTAGE_MIN = 3.4;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

const int HOUR_NUMBERS_X = 24;
const int HOUR_NUMBERS_Y = 23;

const int STYLE_MASK_X = 96;
const int STYLE_MASK_Y = 47;

const int STYLE_SHADOW_X = 91;
const int STYLE_SHADOW_Y = 47;

const int DATE_Y = 136;
const int DAY_Y = 164;

const int STYLE_CORNER_1_X = 96;
const int STYLE_CORNER_1_Y = 100;
const int STYLE_CORNER_2_X = 103;
const int STYLE_CORNER_2_Y = 86;

const int BATTERY_CENTER_X = 100;
const int BATTERY_Y = 175;

const Vector SHADOW[] = {{0,-1}, {1,-1}, {1,-1}, {1,-1}, {2,0}, {2,0}, {-2,0}, {-2,0}, {-1,-1}, {-1,-1}, {-1,-1}, {0,-1}};

const unsigned char* STYLE_SHADOW[12] = {
	epd_bitmap_StyleShadow3, epd_bitmap_StyleShadow4, epd_bitmap_StyleShadow4,
	epd_bitmap_StyleShadow4, epd_bitmap_StyleShadow5, epd_bitmap_StyleShadow5,
  epd_bitmap_StyleShadow1, epd_bitmap_StyleShadow1, epd_bitmap_StyleShadow2,
  epd_bitmap_StyleShadow2, epd_bitmap_StyleShadow2, epd_bitmap_StyleShadow3
};

void SundialWatchy::drawWatchFace()
{
  UiSDK::initScreen(display);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  // This face was authored primarily as white elements on a black background,
  // with some black text/strokes drawn over white regions.
  UiSDK::setTextColor(display, bgColor);


  Vector shadowOffset = SHADOW[currentTime.Hour %12];

  drawBitmap(0,0, epd_bitmap_BG, 200, 200, fgColor, true);
  drawBitmap(0 + shadowOffset.x, 0 + shadowOffset.y, epd_bitmap_FG, 200, 200, bgColor, true);
  UiSDK::drawBitmap(display, 0,0, epd_bitmap_FG, 200, 200, fgColor);

  drawBitmap(HOUR_NUMBERS_X + shadowOffset.x, HOUR_NUMBERS_Y + shadowOffset.y, epd_bitmap_HourNumbers, 152, 81, bgColor, true);

  UiSDK::drawBitmap(display, HOUR_NUMBERS_X, HOUR_NUMBERS_Y, epd_bitmap_HourNumbers, 152, 81, bgColor);
  
  drawTime();

  UiSDK::drawBitmap(display, STYLE_MASK_X, STYLE_MASK_Y, epd_bitmap_StyleMask, 8, 54, fgColor);

  UiSDK::drawBitmap(display, STYLE_SHADOW_X, STYLE_SHADOW_Y, STYLE_SHADOW[currentTime.Hour %12], 18, 55, bgColor);

  drawDate();

  int batState = getBatteryFill(10);

  drawBattery(BATTERY_CENTER_X, BATTERY_Y, bgColor, true, batState);
  drawBattery(BATTERY_CENTER_X + shadowOffset.x, BATTERY_Y + shadowOffset.y, fgColor, true, batState);
  drawBattery(BATTERY_CENTER_X, BATTERY_Y, bgColor, false, batState);
}

void SundialWatchy::drawBattery(int x, int y, uint16_t color, bool even, int batState)
{
  const int segmentWidth = 16;

  int startX = x - segmentWidth * batState / 2;

  drawBitmap(startX - 6, y, epd_bitmap_Sundial_battery_L, 6, 12, color, even);

  for (int i = 0; i < batState; i++)
    drawBitmap(startX + i * segmentWidth, y, epd_bitmap_Sundial_battery_mid, 16, 12, color, even);

  drawBitmap(100 + segmentWidth * batState / 2, y, epd_bitmap_Sundial_battery_R, 6, 12, color, even);
}

void SundialWatchy::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, uint16_t color, bool even) {

  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  UiSDK::startWrite(display);
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      if (i & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
      if (byte & 0x80 && ((x + i + y) % 2 == 0) == even && x + i >= 0 && x + i < 200 && y >= 0 && y < 200)
        UiSDK::writePixel(display, x + i, y, color);
    }
  }
  UiSDK::endWrite(display);
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

static String numberToRoman(int number)
{
  String out = "";
  int num[] = {1,4,5,9,10,40,50,90,100,400,500,900,1000};
  String sym[] = {"I","IV","V","IX","X","XL","L","XC","C","CD","D","CM","M"};
  int i=12;   

  while(number>0)
  {
    int div = number/num[i];
    number = number%num[i];

    while(div--)
    {
      out += sym[i];
    }

    i--;
  }

  return out;
}

void SundialWatchy::drawDate()
{
  UiSDK::setFont(display, &EMPORO9pt7b);

  String dayOfWeek = dayStr(currentTime.Wday);

  String month = monthStr(currentTime.Month);
  String date = month + " " + numberToRoman(currentTime.Day);
  
  int16_t  x1, y1;
  uint16_t w, h;
  UiSDK::getTextBounds(display, date, 0, 0, &x1, &y1, &w, &h);

  UiSDK::setCursor(display, 100 - w / 2 , DATE_Y);
  UiSDK::print(display, date);
  
  UiSDK::setFont(display, &EMPORO13pt7b);
  
  UiSDK::getTextBounds(display, dayOfWeek, 0, 0, &x1, &y1, &w, &h);
  UiSDK::setCursor(display, 100 - w / 2, DAY_Y);
  UiSDK::println(display, dayOfWeek);
}

int SundialWatchy::getBatteryFill(int steps)
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

static double smoothstep(double x) {
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}

void SundialWatchy::drawTime()
{
  int hour = currentTime.Hour;
  int minute = currentTime.Minute;

  int minutes = ((hour + 6) % 12) * 60 + minute;
  double angle = minutes / 4.0;

  Vector hour1 = rotateVector({-100,0}, angle);
  double scale = smoothstep(abs(minutes / 360.0 - 1));

  hour1.x *= 0.7 + 0.3 * scale;
  hour1.y *= 0.7 + 0.3 * scale;

  hour1.x += 100;
  hour1.y += 100;

  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  fillTriangle(STYLE_CORNER_1_X,STYLE_CORNER_2_Y, STYLE_CORNER_1_X,STYLE_CORNER_1_Y, hour1.x,hour1.y, bgColor, true);
  fillTriangle(STYLE_CORNER_1_X,STYLE_CORNER_1_Y, STYLE_CORNER_2_X,STYLE_CORNER_1_Y, hour1.x,hour1.y, bgColor, true);
  fillTriangle(STYLE_CORNER_2_X,STYLE_CORNER_1_Y, STYLE_CORNER_2_X,STYLE_CORNER_2_Y, hour1.x,hour1.y, bgColor, true);
}

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

void SundialWatchy::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                int16_t x2, int16_t y2, uint16_t color, bool even) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }
  if (y1 > y2) {
    _swap_int16_t(y2, y1);
    _swap_int16_t(x2, x1);
  }
  if (y0 > y1) {
    _swap_int16_t(y0, y1);
    _swap_int16_t(x0, x1);
  }

  UiSDK::startWrite(display);
  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    writeFastHLine(a, y0, b - a + 1, color, even);
    UiSDK::endWrite(display);
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    writeFastHLine(a, y, b - a + 1, color, even);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      _swap_int16_t(a, b);
    writeFastHLine(a, y, b - a + 1, color, even);
  }
  UiSDK::endWrite(display);
}

void SundialWatchy::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                 uint16_t color, bool even) {
  UiSDK::startWrite(display);

  for (int i = 0; i < w; i++)
  {
    if (((x + i + y) % 2 == 0) == even)
      UiSDK::drawPixel(display, x + i, y, color);
  }

  UiSDK::endWrite(display);
}

void showWatchFace_Sundial(Watchy &watchy) {
  SundialWatchy face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
