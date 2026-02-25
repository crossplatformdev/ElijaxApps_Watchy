#include "Watchy_pxl999.h"

#include "../../../sdk/UiSDK.h"

#define TIME_FONT timeLGMono42pt7b
#define DATE_FONT timeLGMono20pt7b
#define SMALL_TEXT smTextMono8pt7b


static RTC_DATA_ATTR bool pxl999_showCached = false;
static RTC_DATA_ATTR bool pxl999_pauseEnabled = false;
static RTC_DATA_ATTR bool pxl999_delayedStart = false;
static RTC_DATA_ATTR bool pxl999_runOnce = true;

//Pause weather updates when the watch is not in use to conserve battery life.
//Time is defined in 24h format. Set both times to "0:00" to disable pausing.
static String pxl999_pauseStart = "0:30"; //Stops weather updates at 12:30am
static String pxl999_pauseEnd = "5:45"; //Resumes weather updates at 5:45am

//Night weather icons ;)
static bool pxl999_isNight = false;

//Time between weather syncs in minutes
#define WEATHER_TIMER 30

//Time between NTP syncs in hours
#define NTP_TIMER 12

WatchyPXL999::WatchyPXL999() {}

void WatchyPXL999::drawTime() {

  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  String hourStr = String(currentTime.Hour);
  String minStr = String(currentTime.Minute);

  hourStr = (pxl999_twelve_mode && currentTime.Hour > 12 &&  currentTime.Hour <= 21) ? "0" + String(currentTime.Hour - 12) : (pxl999_twelve_mode && currentTime.Hour > 12) ? String(currentTime.Hour - 12) :
            (pxl999_twelve_mode && currentTime.Hour == 0) ? "12" : currentTime.Hour < 10 ? "0" + hourStr : hourStr;
  minStr = currentTime.Minute < 10 ? "0" + minStr : minStr;

  UiSDK::setFont(display, &TIME_FONT);
  UiSDK::setTextColor(display, fg);

  UiSDK::fillRect(display, 11, 27, 128, 124, bg); //Redraw Helper

  //Hour
  UiSDK::setCursor(display, 16, 87);
  UiSDK::print(display, hourStr);

  //Minute
  UiSDK::setCursor(display, 16, 148);
  UiSDK::print(display, minStr);
}

void WatchyPXL999::drawDate() {
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  String dayName = dayStr(currentTime.Wday);
  String monthName = monthStr(currentTime.Month);
  String dateStr = String(currentTime.Day);
  dateStr = currentTime.Day < 10 ? "0" + dateStr : dateStr;

  UiSDK::fillRect(display, 11, 153, 178, 38, bg); //Redraw Helper

  UiSDK::setFont(display, &DATE_FONT);
  UiSDK::setCursor(display, 16, 184);
  UiSDK::print(display, dateStr);

  UiSDK::setFont(display, &SMALL_TEXT);
  UiSDK::setCursor(display, 76, 169);
  UiSDK::print(display, monthName);

  UiSDK::setCursor(display, 76, 184);
  UiSDK::print(display, dayName);
}

void WatchyPXL999::drawWeatherIcon() {

  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  pxl999_isNight = (currentTime.Hour >= 18 || currentTime.Hour <= 5) ? true : false;
  const unsigned char* weatherIcon;

  //https://openweathermap.org/weather-conditions
  if (pxl999_weatherConditionCode == 999) { //RTC
    weatherIcon = rtc;
  } else if (pxl999_weatherConditionCode == 998) { //RTC SLEEEP
    weatherIcon = rtcsleep;
  } else if (pxl999_weatherConditionCode > 801 && pxl999_weatherConditionCode < 805) { //Cloudy
    weatherIcon = scatteredclouds;
  } else if (pxl999_weatherConditionCode == 801) { //Few Clouds
    weatherIcon = (pxl999_isNight) ? fewcloudsnight : fewclouds;
  } else if (pxl999_weatherConditionCode == 800) { //Clear
    weatherIcon = (pxl999_isNight) ? clearskynight : clearsky;
  } else if (pxl999_weatherConditionCode >= 700) { //Atmosphere
    weatherIcon = mist;
  } else if (pxl999_weatherConditionCode >= 600) { //Snow
    weatherIcon = snow;
  } else if (pxl999_weatherConditionCode >= 500) { //Rain
    weatherIcon = rain;
  } else if (pxl999_weatherConditionCode >= 300) { //Drizzle
    weatherIcon = drizzle;
  } else if (pxl999_weatherConditionCode >= 200) { //Thunderstorm
    weatherIcon = thunderstorm;
  }

  UiSDK::fillRect(display, 141, 91, 49, 44, bg); //Redraw Helper
  UiSDK::drawBitmap(display, 143, 93, weatherIcon, 45, 40, fg);
}

void WatchyPXL999::drawWeather() {
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);

  if (!pauseUpdates() && !pxl999_delayedStart) {
    if (pxl999_debugger)
      Serial.println("Resuming Updates");
    if (pxl999_showCached == false) {
      weatherData latestWeather = getWeather();
      pxl999_temperature = latestWeather.temperature;
      pxl999_weatherConditionCode = latestWeather.weatherConditionCode;
    } else {
      pxl999_showCached = false;
    }
  } else {
    pxl999_temperature = rtcTemp();
    pxl999_latestWeather.temperature = pxl999_temperature;
    pxl999_weatherConditionCode = (pauseUpdates()) ? 998 : 999;
    pxl999_latestWeather.weatherConditionCode = pxl999_weatherConditionCode;
    pxl999_cityNameID = 999;
    if (pxl999_debugger) {
      Serial.println("Paused Updates");
      Serial.println("RTC Temp - latestWeather.temp: " + String(pxl999_latestWeather.temperature));
    }
  }

  UiSDK::setFont(display, &SMALL_TEXT);
  UiSDK::fillRect(display, 142, 136, 49, 13, bg); //Redraw Helper
  //Get width of text & center it under the weather icon. 165 is the centerpoint of the icon
  int16_t  x1, y1;
  uint16_t w, h;
  UiSDK::getTextBounds(display, String(pxl999_temperature) + ".", 45, 13, &x1, &y1, &w, &h);
  UiSDK::setCursor(display, 166 - w / 2, 148);
  if (pxl999_debugger)
    Serial.println("Latest temperature: " + String(pxl999_temperature));
  UiSDK::println(display, String(pxl999_temperature) + ".");

  pxl999_cityName = getCityAbbv();
  UiSDK::fillRect(display, 142, 77, 49, 13, bg); //Redraw Helper
  UiSDK::getTextBounds(display, pxl999_cityName, 45, 13, &x1, &y1, &w, &h);
  UiSDK::setCursor(display, 165 - w / 2, 87);
  if (pxl999_debugger)
    Serial.println("Current City : " + String(pxl999_cityName) + " | " + getCityName());
  UiSDK::println(display, pxl999_cityName);

  if (pxl999_debugger) { //show active weather condition code
    String weathercode = String(pxl999_weatherConditionCode);
    UiSDK::getTextBounds(display, weathercode, 45, 13, &x1, &y1, &w, &h);
    UiSDK::setCursor(display, 165 - w / 2, 55);
    UiSDK::println(display, weathercode);
  }

}

bool WatchyPXL999::pauseUpdates() {

  //Get Times from String
  int sColon = pxl999_pauseStart.indexOf(':');
  int eColon = pxl999_pauseEnd.indexOf(':');

  int pauseHour = (pxl999_pauseStart.substring(0, sColon)).toInt();
  int pauseMin = (pxl999_pauseStart.substring(sColon + 1)).toInt();

  int endHour = (pxl999_pauseEnd.substring(0, eColon)).toInt();
  int endMin = (pxl999_pauseEnd.substring(eColon + 1)).toInt();

  if (currentTime.Hour == pauseHour && currentTime.Minute == pauseMin) {
    pxl999_pauseEnabled = true;
    if (pxl999_debugger)
      Serial.println("Enabling pauseEnabled: " + String(pxl999_pauseEnabled));
  }

  if (currentTime.Hour == endHour && currentTime.Minute == endMin) {
    pxl999_pauseEnabled = false;
    pxl999_runOnce = true;
    if (pxl999_debugger)
      Serial.println("Disabling pauseEnabled: " + String(pxl999_pauseEnabled));
  }

  if (pxl999_debugger) {
    Serial.println("startHour: " + String(pauseHour));
    Serial.println("startMin: " + String(pauseMin));
    Serial.println("endHour: " + String(endHour));
    Serial.println("endMin: " + String(endMin));
    Serial.println("pauseEnabled: " + String(pxl999_pauseEnabled));
  }
  return pxl999_pauseEnabled;
}

void WatchyPXL999::drawWatchFace() {

  UiSDK::initScreen(display);
  drawTime();
  drawDate();

  if (!pauseUpdates() && !pxl999_runOnce) { //Check if live updates aren't paused

    if (pxl999_delayedStart) { //Sync Weather & NTP on second Tick to avoid crashing Watchy on first launch
      pxl999_delayedStart = false;
      drawWeather();
      syncNtpTime();
      if (pxl999_debugger) {
        Serial.println("Delayed Start. Syncing Weather & NTP");
        Serial.println("initial runOnce: " + String(pxl999_runOnce));
        Serial.println("Initial NTP Sync");
      }
    }

    if (currentTime.Minute % WEATHER_TIMER == 0 || currentTime.Hour % NTP_TIMER == 0 && currentTime.Minute == 0) { //Check time to sync Weather or NTP

      if (currentTime.Minute % WEATHER_TIMER == 0) { //Sync Weather
        if (pxl999_debugger)
          Serial.println("getting new weather");
        drawWeather();
        //syncNtpTime();
      }

      if (currentTime.Hour % NTP_TIMER == 0 && currentTime.Minute == 0) { //Sync NTP
        if (pxl999_debugger)
          Serial.println("Getting new NTP time");
        syncNtpTime();
      }

    } else { //Not time to sync, show cached weather
      if (pxl999_debugger)
        Serial.println("showing cached weather");
      pxl999_showCached = true;
      drawWeather();
    }

  } else { //Live updates disabled, show RTC temp and icon
    if (pxl999_runOnce) {
      //this is a SILLY workaround to prevent the watchy from getting stuck in a loop
      //when checking the weather too quickly. I mean, is it silly if it works? XD
      if ((pxl999_startMillis - millis()) >= 3000) {
        pxl999_runOnce = false; //Sync on next tick
      }
      pxl999_delayedStart = true;
      if (pxl999_debugger)
        Serial.println("getting RTC weather first");
      drawWeather();
      if (pxl999_debugger)
        Serial.println("Waiting for delayed start, elapsed millis: " + String(millis()));
    } else {
      if (pxl999_debugger)
        Serial.println("Weather Paused, getting RTC Temp");
      drawWeather();
    }
  }

  drawWeatherIcon();
  if (WiFi.status() == WL_CONNECTED)
    disableWiFi();

  //another silly work around to help reduce ghosting
  for (int i = 0; i < 3; i++) {
    UiSDK::displayUpdate(display, true);
  }

}

void showWatchFace_pxl999(Watchy &watchy) {
  WatchyPXL999 face;
  face.settings = watchy.settings;
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
