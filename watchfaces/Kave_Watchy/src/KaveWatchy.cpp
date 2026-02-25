#include "KaveWatchy.h"

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
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(0, 0, epd_bitmap_BG, 200, 200, GxEPD_BLACK);
  display.setTextColor(GxEPD_BLACK);

  drawTime();
  drawDate();
  drawSteps();
  drawBattery();
}

void KaveWatchy::drawTime()
{
  display.setFont(&NIOBRG__30pt7b);

  bool am = currentTime.Hour < 12;
  int hour = currentTime.Hour;

  if (HOUR_12_24 == 12)
    hour = ((hour+11)%12)+1;
    
  int16_t  x1, y1;
  uint16_t w, h;

  display.getTextBounds(String(":"), 0, 0, &x1, &y1, &w, &h);

  int colonXStart = POS_TIME_CENTER_X - w / 2;
  int colonXEnd = POS_TIME_CENTER_X + w / 2 + 4;

  String hourString = "";
  
  if (hour < 10)
    hourString += "0";
  
  hourString += String(hour);

  display.getTextBounds(hourString, 0, 0, &x1, &y1, &w, &h);

  display.setCursor(colonXStart - w - 4, POS_TIME_Y);
  display.print(hourString);
  
  display.setCursor(colonXStart, POS_TIME_Y);
  display.print(":");

  String minuteString = "";

  if (currentTime.Minute < 10)
    minuteString += "0";

  minuteString += String(currentTime.Minute);

  display.setCursor(colonXEnd, POS_TIME_Y);
  display.print(minuteString);

  if (HOUR_12_24 != 12)
    return;

  display.setFont(&NIOBRG__10pt7b);
  display.setCursor(POS_AM_PM_TIME_X, POS_AM_PM_TIME_Y);
  display.print(am ? "AM" : "PM");
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
  display.setFont(&NIOBRG__10pt7b);

  String dayOfWeek = dayStr(currentTime.Wday);
  String month = monthStr(currentTime.Month);

  display.setCursor(POS_DATE_X, POS_DATE_Y1);
  display.println(dayOfWeek);

  display.setCursor(POS_DATE_X, POS_DATE_Y2);
  display.print(month);
  display.print(" ");

  display.print(currentTime.Day);
  
  display.print(Ordinal(currentTime.Day));
}

void KaveWatchy::drawSteps(){
    // reset step counter at midnight
    if (currentTime.Hour == 0 && currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    
    uint32_t stepCount = sensor.getCounter();

    display.drawBitmap(POS_STEPS_ICON_X, POS_STEPS_ICON_Y, epd_bitmap_Shell, 48, 50, GxEPD_BLACK);
    
    display.drawBitmap(POS_STEPS_BG_X, POS_STEPS_BG_Y, epd_bitmap_Steps_BG, 85, 30, GxEPD_BLACK);

    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(String(stepCount), 0, 0, &x1, &y1, &w, &h);

    display.setFont(&NIOBRG__10pt7b);
    display.setCursor(POS_STEPS_ICON_X - 10 - w, POS_STEPS_BG_Y + POS_STEPS_OFFSET_Y);
    display.println(stepCount);
}

void KaveWatchy::drawBattery()
{
  float VBAT = getBatteryVoltage();

  // 12 battery states
  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * BATTER_FILL_Y);
  if (batState > BATTER_FILL_Y)
    batState = BATTER_FILL_Y;
  if (batState < 0)
    batState = 0;

  int top = random(0, epd_bitmap_Battery_Top_LEN);

  display.drawBitmap(POS_BATTERY_X, POS_BATTERY_Y, epd_bitmap_Battery_BG, 29, 101, GxEPD_BLACK);

  int fillPosY = POS_BATTERY_FILL_Y + (BATTER_FILL_Y - batState);

  int batterTopSizeY = batState;

  if (batterTopSizeY > 20)
    batterTopSizeY = 20;

  display.drawBitmap(POS_BATTERY_FILL_X, fillPosY, epd_bitmap_Battery_Top[top], 25, batterTopSizeY, GxEPD_WHITE);

  const int bubbleSize = 25;

  drawFillBitmap(POS_BATTERY_FILL_X, fillPosY + 2, bubbleSize, batState - 2, random(0, bubbleSize), random(0, bubbleSize), 
  epd_bitmap_Bubbles, bubbleSize, bubbleSize, GxEPD_WHITE);
}


void KaveWatchy::drawFillBitmap(int16_t x, int16_t y, int16_t fw, int16_t fh, int16_t ox, int16_t oy, 
const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;

  display.startWrite();
  for (int16_t fy = 0; fy < fh; fy++, y++) {
    for (int16_t fx = 0; fx < fw; fx++) {

      int16_t bx = (fx + ox) % w;
      int16_t by = (fy + oy) % h;

      if (bx & 7)
        byte <<= 1;
      else
        byte = pgm_read_byte(&bitmap[by * byteWidth + bx / 8]);
      if (byte & 0x80)
        display.writePixel(x + fx, y, color);
    }
  }
  display.endWrite();
}