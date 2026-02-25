#include "BetonWatchy.h"

#include "../../../sdk/UiSDK.h"

const int POS_BATTERY_X = 0;
const int POS_BATTERY_Y = 2;

const int POS_BATTERY_FILL_X = POS_BATTERY_X + 5;
const int POS_BATTERY_FILL_Y = POS_BATTERY_Y + 6;

const int POS_TIME_CENTER_X = 100;
const int POS_TIME_Y = 105;

const int POS_DATE_CENTER_X = 100;
const int POS_DATE_1_Y = 150;

const int POS_DATE_2_Y = 175;

const float VOLTAGE_MIN = 3.4;
const float VOLTAGE_MAX = 4.2;
const float VOLTAGE_RANGE = VOLTAGE_MAX - VOLTAGE_MIN;

void BetonWatchy::drawWatchFace()
{
  UiSDK::initScreen(display);
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  UiSDK::fillScreen(display, bg);
  UiSDK::setTextColor(display, fg);

  drawTime();
  drawDate();
  drawBattery();
}

void BetonWatchy::drawTime()
{
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::setFont(display, &BLKCHCRY40pt7b);

  bool am = currentTime.Hour < 12;
  int hour = currentTime.Hour;

  if (HOUR_12_24 == 12)
    hour = ((hour+11)%12)+1;
    
  int16_t  x1, y1;
  uint16_t w, h;

  String timeString = "";
  
  if (hour < 10)
    timeString += "0";
  
  timeString += String(hour);

  timeString += ":";

  if (currentTime.Minute < 10)
    timeString += "0";

  timeString += String(currentTime.Minute);

  UiSDK::getTextBounds(display, timeString, 0, 0, &x1, &y1, &w, &h);

  UiSDK::setCursor(display, POS_TIME_CENTER_X - w / 2, POS_TIME_Y);
  UiSDK::print(display, timeString);

  UiSDK::drawFastHLine(display, POS_TIME_CENTER_X - w / 2 - 15, POS_TIME_Y, w + 30, fg);
  UiSDK::drawFastHLine(display, POS_TIME_CENTER_X - w / 2 - 5, POS_TIME_Y + 1, w + 10, fg);

  if (HOUR_12_24 != 12)
    return;

  UiSDK::setFont(display, &BLKCHCRY12pt7b);
  UiSDK::setCursor(display, POS_TIME_CENTER_X + w / 2, POS_TIME_Y);
  UiSDK::print(display, am ? "AM" : "PM");
}

const char* BetonWatchy::Ordinal(uint8_t num)
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

void BetonWatchy::drawDate()
{
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::setFont(display, &BLKCHCRY12pt7b);

  String dayOfWeek = dayStr(currentTime.Wday);
  String month = monthStr(currentTime.Month);
  
  int16_t  x1, y1;
  uint16_t w, h;
  
  UiSDK::getTextBounds(display, dayOfWeek, 0, 0, &x1, &y1, &w, &h);

  UiSDK::setCursor(display, POS_DATE_CENTER_X - w / 2, POS_DATE_1_Y);
  UiSDK::println(display, dayOfWeek);

  String day = String(currentTime.Day) + String(Ordinal(currentTime.Day));
  String date = day + String("    ") + month;

  UiSDK::getTextBounds(display, date, 0, 0, &x1, &y1, &w, &h);

  int width = w;

  UiSDK::setCursor(display, POS_DATE_CENTER_X - w / 2, POS_DATE_2_Y);
  UiSDK::print(display, date);

  UiSDK::drawFastHLine(display, POS_DATE_CENTER_X - w / 2 - 10, POS_DATE_2_Y + 1, w + 20, fg);
  
  UiSDK::getTextBounds(display, day, 0, 0, &x1, &y1, &w, &h);
  
  UiSDK::setCursor(display, POS_DATE_CENTER_X - width / 2 + w + 4, POS_DATE_2_Y + 10);
  UiSDK::print(display, "of");
}

void BetonWatchy::drawBattery()
{
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  float VBAT = getBatteryVoltage();

  // 12 battery states
  int batState = int(((VBAT - VOLTAGE_MIN) / VOLTAGE_RANGE) * 191);
  if (batState > 191)
    batState = 191;
  if (batState < 0)
    batState = 0;

  UiSDK::fillRect(display, POS_BATTERY_FILL_X, POS_BATTERY_FILL_Y, batState, 25, fg);

  UiSDK::drawBitmap(display, POS_BATTERY_FILL_X, POS_BATTERY_FILL_Y, epd_bitmap_Battery_Mask_2, 191, 25, bg);

  UiSDK::drawBitmap(display, POS_BATTERY_X, POS_BATTERY_Y, epd_bitmap_Battery_BG_2, 200, 45, fg);
}

void showWatchFace_Castle_of_Watchy(Watchy &watchy) {
  BetonWatchy face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
