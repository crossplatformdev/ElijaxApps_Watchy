#include "x.h"
#include "sdk/UiSDK.h"

void X::drawWatchFace(){
  UiSDK::initScreen(display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  (void)bgColor;

  UiSDK::setTextColor(display, fgColor);
  UiSDK::drawBitmap(display, 45, 38, x, 110, 124, fgColor);
    drawTime();
    drawWDay();
    drawDate();
    drawSteps();
    drawTemperature();
    drawBattery();

    if(currentTime.Hour < 22 && currentTime.Hour > 7 && currentTime.Minute == 0){
        vibMotor(100,2);
    }

    for(uint8_t i=0; i<3; i++){
        // Reduce ghosting
        UiSDK::displayUpdate(display, true);
    }
}

void X::drawTime(){
    int16_t  x1, y1;
    uint16_t w, h;
    String hour;
    String minutes;
    UiSDK::setFont(display, &UnscreenMK27pt7b);
    hour = currentTime.Hour;

    UiSDK::getTextBounds(display, hour, 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 8, 107);
    if (currentTime.Hour < 10){
      UiSDK::print(display, "0" + hour);
    } else {
      UiSDK::print(display, hour);
    }
    
    minutes = currentTime.Minute;
    UiSDK::getTextBounds(display, minutes, 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 125, 107);
    if (currentTime.Minute < 10){
      UiSDK::print(display, "0" + minutes);
    } else {
      UiSDK::print(display, minutes);
    }
    

}

void X::drawWDay(){
    UiSDK::setFont(display, &UnscreenMK8pt7b);
    UiSDK::setCursor(display, 8, 17);
    String dayOfWeek = dayStr(currentTime.Wday);
    dayOfWeek.toUpperCase();
    UiSDK::print(display, String(dayOfWeek));
}

void X::drawDate(){
    UiSDK::setFont(display, &UnscreenMK8pt7b);
    UiSDK::setCursor(display, 8, 30);
    String monthStr = String(currentTime.Month);
    String dayStr = String(currentTime.Day);
    monthStr = currentTime.Month < 10 ? "0" + monthStr : monthStr;
    dayStr = currentTime.Day < 10 ? "0" + dayStr : dayStr;
    String dateStr = monthStr + "." + dayStr;
    UiSDK::print(display, String(dateStr));
}

void X::drawSteps(){
    int16_t  x1, y1;
    uint16_t w, h;
    UiSDK::setFont(display, &UnscreenMK20pt7b);
    uint32_t stepCount = sensor.getCounter();
    String stepStr = String(stepCount);
    for(int i=1; i<5; i++){
        stepStr = stepCount < pow(10, i) ? "0" + stepStr : stepStr;
    }
    // Step counter managed by core Watchy
    UiSDK::getTextBounds(display, stepStr, 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 100-w/2, 185);
    UiSDK::print(display, String(stepStr));
}

void X::drawTemperature(){
    UiSDK::setFont(display, &UnscreenMK8pt7b);
    UiSDK::setCursor(display, 155, 30);
    uint8_t temperatureRTC = temperatureRTC = RTC.temperature() / 4;
    if (settings.weatherUnit == "imperial") {
      temperatureRTC = temperatureRTC * (9/5) + 32;
    }
    if(temperatureRTC < 10){
    UiSDK::print(display, "0");
    }
    UiSDK::print(display, temperatureRTC);
    if (settings.weatherUnit == "imperial") {
      UiSDK::print(display, "Fh");
    } else {
      UiSDK::print(display, "c");
    }
}

void X::drawBattery(){
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::setTextColor(display, fgColor);
    UiSDK::setFont(display, &UnscreenMK8pt7b);
    UiSDK::setCursor(display, 187, 17);
    UiSDK::print(display, ">");
    UiSDK::setCursor(display, 158, 17);
    float BATTV = getBatteryVoltage();
    if(BATTV > 4.10){
    UiSDK::print(display, "xxx");
    }
    else if(BATTV > 3.85 && BATTV <= 4.10){
        UiSDK::print(display, "   xx");
    }
    else if(BATTV > 3.60 && BATTV <= 3.85){
        UiSDK::print(display, "      x");
    }
}

void showWatchFace_X_marks_the_spot(Watchy &watchy) {
  X face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
