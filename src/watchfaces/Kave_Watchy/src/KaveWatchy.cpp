#include "KaveWatchy.h"

#include "../../../sdk/UiSDK.h"

const int POS_BATTERY_X = 15;
const int POS_BATTERY_Y = 15;

const int POS_BATTERY_FILL_X = POS_BATTERY_X + 2;
const int POS_BATTERY_FILL_Y = POS_BATTERY_Y + 14;

const int POS_TIME_CENTER_X = 120;
const int POS_TIME_Y = 120;

const int POS_AM_PM_TIME_X = 160;
const int POS_AM_PM_TIME_Y = 140;

const int POS_DATE_X = 25;
const int POS_DATE_Y1 = 150;

const int POS_DATE_Y2 = 175;

const int POS_STEPS_ICON_X = 137;
const int POS_STEPS_ICON_Y = 15;

const int POS_STEPS_BG_X = 51;
const int POS_STEPS_BG_Y = 25;

const int POS_STEPS_OFFSET_Y = 22;

const int BATTER_FILL_Y = 57;

const float VOLTAGE_MIN = 3.4;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

const int epd_bitmap_Battery_Top_LEN = 6;
const unsigned char* epd_bitmap_Battery_Top[epd_bitmap_Battery_Top_LEN] = {
	epd_bitmap_Battery_Top_1, epd_bitmap_Battery_Top_2, epd_bitmap_Battery_Top_3,
	epd_bitmap_Battery_Top_4, epd_bitmap_Battery_Top_5, epd_bitmap_Battery_Top_6
};

void KaveWatchy::drawWatchFace()
{
  UiSDK::initScreen(display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::drawBitmap(display, 0, 0, epd_bitmap_BG, 200, 200, fgColor);
  UiSDK::setTextColor(display, fgColor);

  drawTime();
  drawDate();
  drawSteps();
  drawBattery();
}

void KaveWatchy::drawTime()
{
  UiSDK::setFont(display, &NIOBRG__30pt7b);

  bool am = currentTime.Hour < 12;
  int hour = currentTime.Hour;

  if (HOUR_12_24 == 12)
    hour = ((hour+11)%12)+1;
    
  int16_t  x1, y1;
  uint16_t w, h;

  UiSDK::getTextBounds(display, String(":"), 0, 0, &x1, &y1, &w, &h);

  int colonXStart = POS_TIME_CENTER_X - w / 2;
  int colonXEnd = POS_TIME_CENTER_X + w / 2 + 4;

  String hourString = "";
  
  if (hour < 10)
    hourString += "0";
  
  hourString += String(hour);

  UiSDK::getTextBounds(display, hourString, 0, 0, &x1, &y1, &w, &h);

  UiSDK::setCursor(display, colonXStart - w - 4, POS_TIME_Y);
  UiSDK::print(display, hourString);
  
  UiSDK::setCursor(display, colonXStart, POS_TIME_Y);
  UiSDK::print(display, ":");

  String minuteString = "";

  if (currentTime.Minute < 10)
    minuteString += "0";

  minuteString += String(currentTime.Minute);

  UiSDK::setCursor(display, colonXEnd, POS_TIME_Y);
  UiSDK::print(display, minuteString);

  if (HOUR_12_24 != 12)
    return;

  UiSDK::setFont(display, &NIOBRG__10pt7b);
  UiSDK::setCursor(display, POS_AM_PM_TIME_X, POS_AM_PM_TIME_Y);
  UiSDK::print(display, am ? "AM" : "PM");
}

const char* KaveWatchy::Ordinal(uint8_t num)
{
  switch(num % 100)
  {
      case 11:
      case 12:
      case 13:
        return "th";
  }

  switch(num % 10)
  {
      case 1:
        return "st";
      case 2:
        return "nd";
      case 3:
        return "rd";
      default:
        return "th";
  }
}

void KaveWatchy::drawDate()
{
  UiSDK::setFont(display, &NIOBRG__10pt7b);

  String dayOfWeek = dayStr(currentTime.Wday);
  String month = monthStr(currentTime.Month);

  UiSDK::setCursor(display, POS_DATE_X, POS_DATE_Y1);
  UiSDK::println(display, dayOfWeek);

  UiSDK::setCursor(display, POS_DATE_X, POS_DATE_Y2);
  UiSDK::print(display, month);
  UiSDK::print(display, " ");

  UiSDK::print(display, currentTime.Day);
  
  UiSDK::print(display, Ordinal(currentTime.Day));
}

void KaveWatchy::drawSteps(){
    // Step counter managed by core Watchy
    
    uint32_t stepCount = sensor.getCounter();

    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::drawBitmap(display, POS_STEPS_ICON_X, POS_STEPS_ICON_Y, epd_bitmap_Shell, 48, 50, fgColor);
    
    UiSDK::drawBitmap(display, POS_STEPS_BG_X, POS_STEPS_BG_Y, epd_bitmap_Steps_BG, 85, 30, fgColor);

    int16_t  x1, y1;
    uint16_t w, h;
    UiSDK::getTextBounds(display, String(stepCount), 0, 0, &x1, &y1, &w, &h);

    UiSDK::setFont(display, &NIOBRG__10pt7b);
    UiSDK::setCursor(display, POS_STEPS_ICON_X - 10 - w, POS_STEPS_BG_Y + POS_STEPS_OFFSET_Y);
    UiSDK::println(display, stepCount);
}

void KaveWatchy::drawBattery()
{
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  float VBAT = getBatteryVoltage();

  // 12 battery states
  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * BATTER_FILL_Y);
  if (batState > BATTER_FILL_Y)
    batState = BATTER_FILL_Y;
  if (batState < 0)
    batState = 0;

  int top = random(0, epd_bitmap_Battery_Top_LEN);

  UiSDK::drawBitmap(display, POS_BATTERY_X, POS_BATTERY_Y, epd_bitmap_Battery_BG, 29, 101, fgColor);

  int fillPosY = POS_BATTERY_FILL_Y + (BATTER_FILL_Y - batState);

  int batterTopSizeY = batState;

  if (batterTopSizeY > 20)
    batterTopSizeY = 20;

  UiSDK::drawBitmap(display, POS_BATTERY_FILL_X, fillPosY, epd_bitmap_Battery_Top[top], 25, batterTopSizeY, bgColor);

  const int bubbleSize = 25;

  drawFillBitmap(POS_BATTERY_FILL_X, fillPosY + 2, bubbleSize, batState - 2, random(0, bubbleSize), random(0, bubbleSize), 
  epd_bitmap_Bubbles, bubbleSize, bubbleSize, bgColor);
}


void KaveWatchy::drawFillBitmap(int16_t x, int16_t y, int16_t fw, int16_t fh, int16_t ox, int16_t oy, 
const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  UiSDK::startWrite(display);
  for (int16_t fy = 0; fy < fh; fy++, y++) {
    for (int16_t fx = 0; fx < fw; fx++) {

      int16_t bx = (fx + ox) % w;
      int16_t by = (fy + oy) % h;

      if (bx & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[by * byteWidth + bx / 8]);
      if (byte & 0x80)
        UiSDK::writePixel(display, x + fx, y, color);
    }
  }
  UiSDK::endWrite(display);
}

void showWatchFace_Kave_Watchy(Watchy &watchy) {
  KaveWatchy face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}