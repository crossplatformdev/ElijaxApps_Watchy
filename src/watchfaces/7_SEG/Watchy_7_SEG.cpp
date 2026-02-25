#include "Watchy_7_SEG.h"
#include "app/HeartRate.h"
#include "fonts/Seven_Segment10pt7b.h"
#include "fonts/DSEG7_Classic_Regular_15.h"
#include "fonts/DSEG7_Classic_Bold_25.h"
#include "fonts/DSEG7_Classic_Regular_39.h"
#include "icons.h"

namespace {

constexpr uint8_t BATTERY_SEGMENT_WIDTH = 7;
constexpr uint8_t BATTERY_SEGMENT_HEIGHT = 11;
constexpr uint8_t BATTERY_SEGMENT_SPACING = 9;
constexpr uint8_t STEP_COUNTER_CELLS = 4;
constexpr int16_t HEART_CENTER_X = 98;
constexpr int16_t HEART_CENTER_Y = 177;

struct LayoutBox {
  int16_t left;
  int16_t top;
  int16_t right;
  int16_t bottom;
};

constexpr bool isInsideDisplay(LayoutBox box) {
  return box.left >= 0 && box.top >= 0 && box.right <= DISPLAY_WIDTH &&
         box.bottom <= DISPLAY_HEIGHT && box.left < box.right &&
         box.top < box.bottom;
}

constexpr bool overlaps(LayoutBox first, LayoutBox second) {
  return first.left < second.right && second.left < first.right &&
         first.top < second.bottom && second.top < first.bottom;
}
constexpr int16_t heartLeft = HEART_CENTER_X - HEART_ICON_WIDTH / 2;
constexpr int16_t heartRight = HEART_CENTER_X + HEART_ICON_WIDTH / 2;
constexpr LayoutBox timeBox{10, 5, 188, 59};
constexpr LayoutBox weekdayBox{5, 72, 86, 86};
constexpr LayoutBox monthBox{47, 97, 86, 111};
constexpr LayoutBox dayBox{7, 95, 45, 120};
constexpr LayoutBox yearBox{7, 125, 87, 150};
constexpr LayoutBox bluetoothBox{100, 73, 113, 94};
constexpr LayoutBox wifiBox{116, 75, 142, 93};
constexpr LayoutBox chargeBox{142, 75, 158, 93};
constexpr LayoutBox batteryBox{158, 73, 195, 94};
constexpr LayoutBox temperatureBox{87, 100, 159, 158};
constexpr LayoutBox unitBox{165, 110, 191, 130};
constexpr LayoutBox stepIconBox{10, 165, 29, 188};
constexpr LayoutBox stepTextBox{36, 177, 75, 191};
constexpr LayoutBox heartBox{heartLeft,
                             HEART_CENTER_Y - HEART_ICON_HEIGHT / 2,
                             heartRight,
                             HEART_CENTER_Y + HEART_ICON_HEIGHT / 2};
constexpr LayoutBox bpmBox{113, 179, 143, 194};
constexpr LayoutBox weatherBox{145, 158, 193, 190};

constexpr LayoutBox layoutBoxes[] = {
    timeBox,       weekdayBox, monthBox,     dayBox,
    yearBox,       bluetoothBox, wifiBox,    chargeBox,
    batteryBox,    temperatureBox, unitBox,  stepIconBox,
    stepTextBox,   heartBox,     bpmBox,     weatherBox};

template <size_t boxCount>
constexpr bool allBoxesInside(const LayoutBox (&boxes)[boxCount],
                              size_t index = 0) {
  return index == boxCount
             ? true
             : isInsideDisplay(boxes[index]) &&
                   allBoxesInside(boxes, index + 1);
}

template <size_t boxCount>
constexpr bool allBoxesDisjoint(const LayoutBox (&boxes)[boxCount],
                                size_t first = 0, size_t second = 1) {
  return first + 1 >= boxCount
             ? true
             : second >= boxCount
                   ? allBoxesDisjoint(boxes, first + 1, first + 2)
                   : !overlaps(boxes[first], boxes[second]) &&
                         allBoxesDisjoint(boxes, first, second + 1);
}

constexpr size_t bitmapByteCount(size_t width, size_t height) {
  return ((width + 7) / 8) * height;
}

static_assert(allBoxesInside(layoutBoxes),
              "Every 7 SEG element must fit inside the display");
static_assert(allBoxesDisjoint(layoutBoxes),
              "7 SEG elements must never overlap");
static_assert(sizeof(battery) == bitmapByteCount(37, 21) &&
                  sizeof(bluetooth) == bitmapByteCount(13, 21) &&
                  sizeof(wifi) == bitmapByteCount(26, 18) &&
                  sizeof(wifioff) == bitmapByteCount(26, 18) &&
                  sizeof(charge) == bitmapByteCount(16, 18),
              "7 SEG status bitmap dimensions are invalid");
static_assert(sizeof(celsius) == bitmapByteCount(26, 20) &&
                  sizeof(fahrenheit) == bitmapByteCount(26, 20) &&
                  sizeof(cloudsun) == bitmapByteCount(48, 32) &&
                  sizeof(cloudy) == bitmapByteCount(48, 32) &&
                  sizeof(rain) == bitmapByteCount(48, 32) &&
                  sizeof(snow) == bitmapByteCount(48, 32) &&
                  sizeof(sunny) == bitmapByteCount(48, 32) &&
                  sizeof(atmosphere) == bitmapByteCount(48, 32) &&
                  sizeof(drizzle) == bitmapByteCount(48, 32) &&
                  sizeof(thunderstorm) == bitmapByteCount(48, 32) &&
                  sizeof(chip) == bitmapByteCount(48, 32),
              "7 SEG weather bitmap dimensions are invalid");
static_assert(sizeof(steps) == bitmapByteCount(19, 23) &&
                  sizeof(heart) ==
                      bitmapByteCount(HEART_ICON_WIDTH, HEART_ICON_HEIGHT),
              "7 SEG bottom-row bitmap dimensions are invalid");

bool drawTextInBox(const char *text, LayoutBox box, int16_t baseline,
                   bool rightAligned) {
  int16_t xOffset;
  int16_t yOffset;
  uint16_t width;
  uint16_t height;
  Watchy::display.getTextBounds(text, 0, baseline, &xOffset, &yOffset,
                                &width, &height);
  int16_t cursorX = rightAligned
                        ? box.right - xOffset - static_cast<int16_t>(width)
                        : box.left - xOffset;
  int32_t textLeft = cursorX + xOffset;
  int32_t textRight = textLeft + width;
  int32_t textBottom = static_cast<int32_t>(yOffset) + height;
  if (textLeft < box.left || textRight > box.right || yOffset < box.top ||
      textBottom > box.bottom) {
    return false;
  }
  Watchy::display.setCursor(cursorX, baseline);
  Watchy::display.print(text);
  return true;
}

} // namespace

void Watchy7SEG::drawWatchFace(){
    display.setTextSize(1);
    display.setTextWrap(false);
    drawTime();
    drawDate();
    drawSteps();
    drawWeather();
    drawBattery();
    drawHeartIcon(HEART_CENTER_X, HEART_CENTER_Y, heartRateValid,
            DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.setFont(&Seven_Segment10pt7b);
    char bpmText[4];
    if(heartRateValid){
      snprintf(bpmText, sizeof(bpmText), "%u", heartRateBpm);
    }else{
      snprintf(bpmText, sizeof(bpmText), "--");
    }
    drawTextInBox(bpmText, bpmBox, 193, false);
    display.drawBitmap(wifiBox.left, wifiBox.top,
               WIFI_CONFIGURED ? wifi : wifioff,
               wifiBox.right - wifiBox.left,
               wifiBox.bottom - wifiBox.top,
               DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    if(BLE_CONFIGURED){
      display.drawBitmap(bluetoothBox.left, bluetoothBox.top, bluetooth,
                 bluetoothBox.right - bluetoothBox.left,
                 bluetoothBox.bottom - bluetoothBox.top,
                 DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
    #ifdef ARDUINO_ESP32S3_DEV
    if(USB_PLUGGED_IN){
      display.drawBitmap(chargeBox.left, chargeBox.top, charge,
               chargeBox.right - chargeBox.left,
               chargeBox.bottom - chargeBox.top,
               DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
    #endif
}

void Watchy7SEG::drawTime(){
    display.setFont(&DSEG7_Classic_Bold_53);
    int displayHour;
    if(HOUR_12_24==12){
      displayHour = ((currentTime.Hour+11)%12)+1;
    } else {
      displayHour = currentTime.Hour;
    }
    char timeText[6];
    snprintf(timeText, sizeof(timeText), "%02d:%02d", displayHour,
         static_cast<int>(currentTime.Minute));
    drawTextInBox(timeText, timeBox, 58, false);
}

void Watchy7SEG::drawDate(){
    display.setFont(&Seven_Segment10pt7b);

    String dayOfWeek = dayStr(currentTime.Wday);
  if (!drawTextInBox(dayOfWeek.c_str(), weekdayBox, 85, true)) {
    dayOfWeek = dayShortStr(currentTime.Wday);
    drawTextInBox(dayOfWeek.c_str(), weekdayBox, 85, true);
    }

    String month = monthShortStr(currentTime.Month);
  drawTextInBox(month.c_str(), monthBox, 110, true);

    display.setFont(&DSEG7_Classic_Bold_25);
  char dayText[3];
  snprintf(dayText, sizeof(dayText), "%02d",
       static_cast<int>(currentTime.Day));
  drawTextInBox(dayText, dayBox, 120, false);
  char yearText[5];
  snprintf(yearText, sizeof(yearText), "%04d",
       tmYearToCalendar(currentTime.Year));
  drawTextInBox(yearText, yearBox, 150, false);
}
void Watchy7SEG::drawSteps(){
    // reset step counter at midnight
    if (currentTime.Hour == 0 && currentTime.Minute == 0){
      sensor.resetStepCounter();
    }
    uint32_t stepCount = sensor.getCounter();
    display.setFont(&Seven_Segment10pt7b);
    display.setTextSize(1);
    display.drawBitmap(stepIconBox.left, stepIconBox.top, steps,
           stepIconBox.right - stepIconBox.left,
           stepIconBox.bottom - stepIconBox.top,
               DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    char stepText[STEP_COUNTER_CELLS + 1];
    if(stepCount < 10000){
      snprintf(stepText, sizeof(stepText), "%04lu",
           static_cast<unsigned long>(stepCount));
    }else{
      unsigned long thousands = stepCount / 1000;
      if(thousands > 999){
        thousands = 999;
      }
      snprintf(stepText, sizeof(stepText), "%3luK", thousands);
    }
    drawTextInBox(stepText, stepTextBox, 190, true);
}
void Watchy7SEG::drawBattery(){
  display.drawBitmap(batteryBox.left, batteryBox.top, battery,
                     batteryBox.right - batteryBox.left,
                     batteryBox.bottom - batteryBox.top,
                     DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    display.fillRect(163, 78, 27, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_BLACK : GxEPD_WHITE);//clear battery segments
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 3.80){
        batteryLevel = 3;
    }
    else if(VBAT > 3.6 && VBAT <= 3.80){
        batteryLevel = 2;
    }
    else if(VBAT > 3.20 && VBAT <= 3.6){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.20){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        display.fillRect(163 + (batterySegments * BATTERY_SEGMENT_SPACING), 78, BATTERY_SEGMENT_WIDTH, BATTERY_SEGMENT_HEIGHT, DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    }
}

bool Watchy7SEG::updateWatchFaceData() {
    weatherData previousWeather = getCachedWeatherData();
    weatherData updatedWeather = getWeatherData();
    return previousWeather.temperature != updatedWeather.temperature ||
           previousWeather.weatherConditionCode !=
               updatedWeather.weatherConditionCode ||
           previousWeather.isMetric != updatedWeather.isMetric;
}

void Watchy7SEG::drawWeather(){

    weatherData currentWeather = getCachedWeatherData();

    int8_t temperature = currentWeather.temperature;
    int16_t weatherConditionCode = currentWeather.weatherConditionCode;
    bool weatherValid = weatherConditionCode >= 200 &&
                        weatherConditionCode <= 804;

    char temperatureText[5];
    if(weatherValid){
      snprintf(temperatureText, sizeof(temperatureText), "%d",
               static_cast<int>(temperature));
    }else{
      snprintf(temperatureText, sizeof(temperatureText), "--");
    }
    display.setFont(&DSEG7_Classic_Regular_39);
    bool temperatureDrawn =
        drawTextInBox(temperatureText, temperatureBox, 150, true);
    if (!temperatureDrawn) {
      display.setFont(&DSEG7_Classic_Bold_25);
      temperatureDrawn =
          drawTextInBox(temperatureText, temperatureBox, 136, true);
    }
    if (!temperatureDrawn) {
      snprintf(temperatureText, sizeof(temperatureText), "--");
      drawTextInBox(temperatureText, temperatureBox, 136, true);
    }
    display.drawBitmap(unitBox.left, unitBox.top,
               currentWeather.isMetric ? celsius : fahrenheit,
               unitBox.right - unitBox.left,
               unitBox.bottom - unitBox.top,
               DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
    const unsigned char* weatherIcon;

    if(WIFI_CONFIGURED && weatherValid){
      //https://openweathermap.org/weather-conditions
      if(weatherConditionCode > 801){//Cloudy
        weatherIcon = cloudy;
      }else if(weatherConditionCode == 801){//Few Clouds
        weatherIcon = cloudsun;
      }else if(weatherConditionCode == 800){//Clear
        weatherIcon = sunny;
      }else if(weatherConditionCode >=700){//Atmosphere
        weatherIcon = atmosphere;
      }else if(weatherConditionCode >=600){//Snow
        weatherIcon = snow;
      }else if(weatherConditionCode >=500){//Rain
        weatherIcon = rain;
      }else if(weatherConditionCode >=300){//Drizzle
        weatherIcon = drizzle;
      }else if(weatherConditionCode >=200){//Thunderstorm
        weatherIcon = thunderstorm;
      }else{
        weatherIcon = chip;
      }
    }else{
      weatherIcon = chip;
    }
    
    display.drawBitmap(weatherBox.left, weatherBox.top, weatherIcon,
               weatherBox.right - weatherBox.left,
               weatherBox.bottom - weatherBox.top,
               DARKMODE ? GxEPD_WHITE : GxEPD_BLACK);
}