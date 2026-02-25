//Derived from peerdavid's source at: https://github.com/peerdavid/Watchy
#include "Watchy_Base.h"
#include "wifi999.h"

RTC_DATA_ATTR weatherData pxl999_latestWeather;
RTC_DATA_ATTR bool pxl999_twelve_mode = true;
static RTC_DATA_ATTR bool pxl999_sleep_mode = false;

RTC_DATA_ATTR int8_t pxl999_temperature;
RTC_DATA_ATTR int16_t pxl999_weatherConditionCode;
RTC_DATA_ATTR unsigned long pxl999_startMillis;

RTC_DATA_ATTR int pxl999_cityNameID = 999;
RTC_DATA_ATTR String pxl999_cityName;
RTC_DATA_ATTR char pxl999_city = 0;

uint16_t ambientOffset = 11.; //This varies for every RTC. Mine runs 11° F hotter than outside the watch.

//Set this flag to true if you want to monitor Serial logs
RTC_DATA_ATTR bool pxl999_debugger = false;

Pxl999WatchyBase::Pxl999WatchyBase() {}

void Pxl999WatchyBase::init() {
  
  pxl999_startMillis = millis();

  if (pxl999_debugger)
    Serial.begin(115200);

  wakeup_reason = esp_sleep_get_wakeup_cause(); //get wake up reason

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm

      // Handle classical tick
      RTC.clearAlarm();

      if (guiState == WATCHFACE_STATE) {
        RTC.read(currentTime);
        showWatchFace(true); //partial updates on tick
      }
      break;

    case ESP_SLEEP_WAKEUP_EXT1: //button Press + no handling if wakeup
      if (pxl999_sleep_mode) {
        pxl999_sleep_mode = false;
        RTC.clearAlarm();

        RTC.read(currentTime);
        showWatchFace(false); //full update on wakeup from sleep mode
        break;
      }

      handleButtonPress();
      break;

    default: //reset
      _rtcConfig();
      showWatchFace(true); //full update on reset
      break;
  }

  deepSleep();
}

void Pxl999WatchyBase::deepSleep() {
  Watchy::deepSleep();
}

void Pxl999WatchyBase::handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

  if (IS_BTN_LEFT_UP) {
    pxl999_twelve_mode = (pxl999_twelve_mode == 0) ? true : false;
    RTC.read(currentTime);
    vibrate();
    showWatchFace(true);
    return;
  }

  Watchy::handleButtonPress();

}

void Pxl999WatchyBase::vibrate(uint8_t times, uint32_t delay_time) {
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  for (uint8_t i = 0; i < times; i++) {
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, true);
    delay(delay_time);
    digitalWrite(VIB_MOTOR_PIN, false);
  }
}

void Pxl999WatchyBase::_rtcConfig() {
  RTC.init();
  RTC.clearAlarm();
  RTC.read(currentTime);
}

uint16_t Pxl999WatchyBase::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t Pxl999WatchyBase::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 !=  Wire.endTransmission());
}

bool Pxl999WatchyBase::noAlpha(String str) { //Check if the city name is an ID code or a text name
  if (str.length() == 0) {
    return false;
  }

  for (int i = 0; i < str.length(); i++) {
    if (str[i] < '0' || str[i] > '9') {
      return false;
    }
  }

  return true;
}

int Pxl999WatchyBase::rtcTemp() {
  pxl999_temperature = RTC.temperature() / 4; //celsius
  if (strcmp(TEMP, "imperial") == 0) {
    pxl999_temperature = (pxl999_temperature * 9. / 5. + 32.) - ambientOffset; //fahrenheit
  }
  if (pxl999_debugger)
    Serial.println("rtcTemp(): " + String(pxl999_temperature));
  return pxl999_temperature;
}

weatherData Pxl999WatchyBase::getWeather() {

  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();

  if (WiFi.status() == WL_CONNECTED) { //Use Weather API for live data if WiFi is connected

    HTTPClient http;
    http.setConnectTimeout(3000);//3 second max timeout

    String weatherQueryURL = (noAlpha(getCityName())) ? String(URL) + String("?id=") + String(getCityName())
                             : String(URL) + String("?q=") + String(getCityName()) + String(",") + String(COUNTRY);
    weatherQueryURL = weatherQueryURL + String("&units=") + String(TEMP) + String("&appid=") + String(APIKEY);

    if (pxl999_debugger) {
      Serial.print("CITY NAME OR ID: ");
      Serial.println(noAlpha(getCityName()) ? "ID" : "NAME");
      Serial.println("weatherQueryURL=" + weatherQueryURL);
    }

    http.begin(weatherQueryURL.c_str());
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar responseObject = JSON.parse(payload);
      pxl999_latestWeather.temperature = int(responseObject["main"]["temp"]);
      pxl999_latestWeather.weatherConditionCode = int(responseObject["weather"][0]["id"]);

    } else {
      //http error
    }

    http.end();

  } else {
    //No WiFi, use RTC Temperature
    pxl999_temperature = rtcTemp();
    pxl999_latestWeather.temperature = pxl999_temperature;
    if (pxl999_debugger) {
      Serial.println("No WiFi, getting RTC Temp");
      Serial.println("latestWeather.temperature: " + String(pxl999_latestWeather.temperature));
    }
    pxl999_weatherConditionCode = 999;
    pxl999_latestWeather.weatherConditionCode = pxl999_weatherConditionCode;
    pxl999_cityNameID = 999;
  }

  return pxl999_latestWeather;
}

//NTP derived from a mashup of Symptym's snippet posted to the Watchy Discord
//https://gist.github.com/Symptym/0336f3f3d74dc66fe05e8d232bed3704
//and help fron Aliceafterall's NTP example: https://github.com/aliceafterall/Watchy/
void Pxl999WatchyBase::syncNtpTime() {

  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();

  if (WiFi.status() == WL_CONNECTED) {

    if (pxl999_debugger)
      Serial.println("Checking NTP time");
    bool syncFailed = false;

    configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
    setenv("TZ", TIMEZONE_STRING, 1);

    int i = 0;
    while (!sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED && i < 10) {
      if (pxl999_debugger)
        Serial.print(".");
      delay(1000);
      i++;
      if (i == 10) {
        if (pxl999_debugger)
          Serial.println("\nFailed to sync");
        syncFailed = true;
      }
    }

    if (!syncFailed) {
      time_t tnow = time(nullptr);
      struct tm *local = localtime(&tnow);

      if (pxl999_debugger) {
        Serial.println("\nNTP Retrieved");
        Serial.print("Date: ");
        Serial.print(local->tm_year + 1900);
        Serial.print("-");
        if ((local->tm_mon + 1) < 10) {
          Serial.print("0");
        }
        Serial.print(local->tm_mon + 1);
        Serial.print("-");
        if (local->tm_mday < 10) {
          Serial.print("0");
        }
        Serial.println(local->tm_mday);
        Serial.print("Time: ");
        if (local->tm_hour < 10) {
          Serial.print("0");
        }
        Serial.print(local->tm_hour);
        Serial.print(":");
        if (local->tm_min < 10) {
          Serial.print("0");
        }
        Serial.print(local->tm_min);
        Serial.print(":");
        if (local->tm_sec < 10) {
          Serial.print("0");
        }
        Serial.println(local->tm_sec);
        Serial.print("Week Day: ");
        Serial.println(local->tm_wday);
      }
      
      currentTime.Year = local->tm_year + YEAR_OFFSET - 2040; //This change matches watchy defaults
      currentTime.Month = local->tm_mon + 1;
      currentTime.Day = local->tm_mday;
      currentTime.Hour = local->tm_hour;
      currentTime.Minute = local->tm_min;
      currentTime.Second = local->tm_sec;
      currentTime.Wday = local->tm_wday + 1;
      RTC.set(currentTime);
      RTC.read(currentTime);
      
      if (pxl999_debugger)
        Serial.println("NTP Sync Done");

    } else {
      Serial.println("NTP Sync Failed");
    }
  }
}
