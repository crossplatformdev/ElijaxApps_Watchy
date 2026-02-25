/*
 * Chaos - Lorenz Attractor Watch Face for Watchy
 * 
 * Copyright (c) 2024 Ajey Pai Karkala
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * A mesmerizing watch face for the Watchy ESP32 smartwatch featuring a real-time
 * evolving Lorenz attractor pattern. Watch the beautiful chaos unfold on your wrist
 * with a continuously updating 3D trajectory that rotates and evolves.
 */

#define ARDUINO_WATCHY_V20
#include "../../../src/watchy/Watchy.h"
#include "../../../src/settings/settings.h"
#include "esp_sleep.h"
#include <math.h>

#ifndef BTN_PIN_MASK
  #ifndef MENU_BTN_PIN
    #define MENU_BTN_PIN 26
  #endif
  #ifndef BACK_BTN_PIN
    #define BACK_BTN_PIN 25
  #endif
  #ifndef UP_BTN_PIN
    #define UP_BTN_PIN 32
  #endif
  #ifndef DOWN_BTN_PIN
    #define DOWN_BTN_PIN 4
  #endif
  #define BTN_PIN_MASK ((1ULL << MENU_BTN_PIN) | (1ULL << BACK_BTN_PIN) | (1ULL << UP_BTN_PIN) | (1ULL << DOWN_BTN_PIN))
#endif

RTC_DATA_ATTR float g_pos[3] = {1.0, 1.0, 1.0};
RTC_DATA_ATTR float g_trajectory[LORENZ_MAX_POINTS][3];
RTC_DATA_ATTR int g_pointCount = 0;
RTC_DATA_ATTR int g_trajectoryIndex = 0;
RTC_DATA_ATTR float g_rotationAngle = 0.0;
RTC_DATA_ATTR int g_updateCounter = 0;

extern bool alreadyInMenu;
extern long gmtOffset;
extern tmElements_t bootTime;

class WatchFace : public Watchy {
  using Watchy::Watchy;
  public:
    void init(String datetime = "") {
      esp_sleep_wakeup_cause_t wakeup_reason;
      wakeup_reason = esp_sleep_get_wakeup_cause();
      #ifdef ARDUINO_ESP32S3_DEV
        Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL);
      #else
        Wire.begin(SDA, SCL);
      #endif
      RTC.init();
      display.epd2.initWatchy();

      switch (wakeup_reason) {
      case ESP_SLEEP_WAKEUP_TIMER:
        RTC.read(currentTime);
        switch (guiState) {
        case WATCHFACE_STATE: {
          g_updateCounter++;
          // Full refresh periodically to prevent ghosting
          // Partial refresh otherwise for smoother animation and battery saving
          bool fullRefresh = (g_updateCounter >= FULL_REFRESH_INTERVAL);
          if (fullRefresh) {
            g_updateCounter = 0;
          }
          showWatchFace(fullRefresh);
          if (settings.vibrateOClock) {
            if (currentTime.Minute == 0 && currentTime.Second == 0) {
              vibMotor(75, 4);
            }
          }
          break;
        }
        case MAIN_MENU_STATE:
          if (alreadyInMenu) {
            guiState = WATCHFACE_STATE;
            showWatchFace(false);
          } else {
            alreadyInMenu = true;
          }
          break;
        }
        break;
      #ifndef ARDUINO_ESP32S3_DEV
      case ESP_SLEEP_WAKEUP_EXT0:
        RTC.read(currentTime);
        switch (guiState) {
        case WATCHFACE_STATE: {
          g_updateCounter++;
          // Full refresh periodically to prevent ghosting
          bool fullRefresh = (g_updateCounter >= FULL_REFRESH_INTERVAL);
          if (fullRefresh) {
            g_updateCounter = 0;
          }
          showWatchFace(fullRefresh);
          if (settings.vibrateOClock) {
            if (currentTime.Minute == 0) {
              vibMotor(75, 4);
            }
          }
          break;
        }
        case MAIN_MENU_STATE:
          if (alreadyInMenu) {
            guiState = WATCHFACE_STATE;
            showWatchFace(false);
          } else {
            alreadyInMenu = true;
          }
          break;
        }
        break;
      #endif
      case ESP_SLEEP_WAKEUP_EXT1:
        RTC.read(currentTime);
        handleButtonPress();
        break;
      #ifdef ARDUINO_ESP32S3_DEV
      case ESP_SLEEP_WAKEUP_EXT0: {
        pinMode(USB_DET_PIN, INPUT);
        USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
        if(guiState == WATCHFACE_STATE){
          RTC.read(currentTime);
          g_updateCounter++;
          // Full refresh periodically to prevent ghosting
          bool fullRefresh = (g_updateCounter >= FULL_REFRESH_INTERVAL);
          if (fullRefresh) {
            g_updateCounter = 0;
          }
          showWatchFace(fullRefresh);
        }
        break;
      }
      #endif
      default:
        Watchy::init(datetime);
        break;
      }
      deepSleep();
    }
    
    void deepSleep() {
      display.hibernate();
      RTC.clearAlarm();
      
      #ifdef ARDUINO_ESP32S3_DEV
      esp_sleep_enable_ext0_wakeup((gpio_num_t)USB_DET_PIN, USB_PLUGGED_IN ? LOW : HIGH);
      rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

      rtc_gpio_set_direction((gpio_num_t)MENU_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_en((gpio_num_t)MENU_BTN_PIN);
      rtc_gpio_set_direction((gpio_num_t)BACK_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_en((gpio_num_t)BACK_BTN_PIN);
      rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);
      rtc_gpio_set_direction((gpio_num_t)DOWN_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
      rtc_gpio_pullup_en((gpio_num_t)DOWN_BTN_PIN);

      esp_sleep_enable_ext1_wakeup(
          BTN_PIN_MASK,
          ESP_EXT1_WAKEUP_ANY_LOW);

      rtc_clk_32k_enable(true);
      
      uint64_t wakeup_us = (uint64_t)(WAKE_UP_INTERVAL_MINUTES * 60.0 * 1000000.0);
      if (wakeup_us < 1000000) wakeup_us = 1000000;
      esp_sleep_enable_timer_wakeup(wakeup_us);
      #else
      const uint64_t ignore = 0b11110001000000110000100111000010;
      for (int i = 0; i < GPIO_NUM_MAX; i++) {
        if ((ignore >> i) & 0b1)
          continue;
        pinMode(i, INPUT);
      }
      #ifdef BTN_PIN_MASK
        esp_sleep_enable_ext1_wakeup(
            BTN_PIN_MASK,
            ESP_EXT1_WAKEUP_ANY_HIGH);
      #endif
      
      uint64_t wakeup_us = (uint64_t)(WAKE_UP_INTERVAL_MINUTES * 60.0 * 1000000.0);
      if (wakeup_us < 1000000) wakeup_us = 1000000;
      esp_sleep_enable_timer_wakeup(wakeup_us);
      #endif
      esp_deep_sleep_start();
    }
    
    void showWatchFace(bool fullRefresh) {
      drawWatchFace();
      display.display(!fullRefresh);
    }
    
    void drawWatchFace() {
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      bool light = true;

      if(currentTime.Hour == 00 && currentTime.Minute == 00) {
        sensor.resetStepCounter();
      }

      display.fillScreen(light ? GxEPD_WHITE : GxEPD_BLACK);
      display.setTextColor(light ? GxEPD_BLACK : GxEPD_WHITE);
      display.setTextWrap(false);

      drawLorenzAttractor();
      
      display.setFont(&FreeMonoBold9pt7b);
      
      if (currentTime.Hour < 10) {
        textstring = "0";
      } else {
        textstring = "";
      }
      textstring += currentTime.Hour;
      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      }
      textstring += currentTime.Minute;
      
      display.setCursor(5, 19);
      display.print(textstring);
      
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      int timeWidth = w;
      int timeEndX = 5 + timeWidth;

      drawMoonPhase();

      display.setFont(&FreeMonoBold9pt7b);
      textstring = dayShortStr(currentTime.Wday);
      textstring += " ";
      textstring += currentTime.Day;
      textstring += "/";
      textstring += currentTime.Month;
      
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      int moonRadius = 10;
      int moonX = 200 - moonRadius - 8;
      int moonLeftEdge = moonX - moonRadius;
      int maxDateX = moonLeftEdge - 5;
      
      int dateStartX = timeEndX + 12;
      
      if (dateStartX + w > maxDateX) {
        dateStartX = maxDateX - w;
        if (dateStartX < timeEndX + 5) {
          dateStartX = timeEndX + 5;
        }
      }
      
      if (dateStartX + w > 200) {
        dateStartX = 200 - w;
      }
      
      display.setCursor(dateStartX, 19);
      display.print(textstring);

      float VBAT = getBatteryVoltage();
      float batteryMin = 3.0;
      float batteryMax = 4.2;
      float batteryPercent = (VBAT - batteryMin) / (batteryMax - batteryMin);
      if (batteryPercent < 0.0) batteryPercent = 0.0;
      if (batteryPercent > 1.0) batteryPercent = 1.0;
      
      int batteryX = 200 - 25;
      int batteryY = 200 - 15;
      int batteryWidth = 20;
      int batteryHeight = 10;
      
      display.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, GxEPD_BLACK);
      display.drawRect(batteryX + batteryWidth, batteryY + 2, 2, 6, GxEPD_BLACK);
      
      int fillWidth = (int)((batteryWidth - 2) * batteryPercent);
      if (fillWidth > 0) {
        display.fillRect(batteryX + 1, batteryY + 1, fillWidth, batteryHeight - 2, GxEPD_BLACK);
      }

      textstring = String(sensor.getCounter());
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(5, 200-15 + 5);
      display.print(textstring);

      drawSunriseSunset();

    }

  private:
    static constexpr float LORENZ_SIGMA = 10.0;
    static constexpr float LORENZ_RHO = 28.0;
    static constexpr float LORENZ_BETA = 8.0/3.0;
    static constexpr float DT = 0.05;
    static constexpr int MAX_POINTS = LORENZ_MAX_POINTS;

    void drawLorenzAttractor() {
      for (int i = 0; i < LORENZ_POINTS_PER_UPDATE; i++) {
        updateLorenz();
        addTrajectoryPoint();
      }
      
      g_rotationAngle += LORENZ_ROTATION_SPEED;
      if (g_rotationAngle > 2 * PI) {
        g_rotationAngle -= 2 * PI;
      }
      
      drawTrajectory();
    }

    void updateLorenz() {
      float k1[3], k2[3], k3[3], k4[3];
      float temp[3];
      
      lorenzDerivative(g_pos, k1);
      
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k1[i] * DT / 2;
      }
      lorenzDerivative(temp, k2);
      
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k2[i] * DT / 2;
      }
      lorenzDerivative(temp, k3);
      
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k3[i] * DT;
      }
      lorenzDerivative(temp, k4);
      
      for (int i = 0; i < 3; i++) {
        g_pos[i] += (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * DT / 6;
      }
    }

    void lorenzDerivative(float p[3], float result[3]) {
      result[0] = LORENZ_SIGMA * (p[1] - p[0]);
      result[1] = p[0] * (LORENZ_RHO - p[2]) - p[1];
      result[2] = p[0] * p[1] - LORENZ_BETA * p[2];
    }

    void addTrajectoryPoint() {
      if (g_pointCount < MAX_POINTS) {
        g_trajectory[g_trajectoryIndex][0] = g_pos[0];
        g_trajectory[g_trajectoryIndex][1] = g_pos[1];
        g_trajectory[g_trajectoryIndex][2] = g_pos[2];
        g_trajectoryIndex++;
        g_pointCount++;
      } else {
        g_trajectory[g_trajectoryIndex][0] = g_pos[0];
        g_trajectory[g_trajectoryIndex][1] = g_pos[1];
        g_trajectory[g_trajectoryIndex][2] = g_pos[2];
        g_trajectoryIndex = (g_trajectoryIndex + 1) % MAX_POINTS;
        g_pointCount = MAX_POINTS;
      }
    }

    void project3DTo2D(float point3D[3], float& x2D, float& y2D) {
      float cos_rot = cos(g_rotationAngle);
      float sin_rot = sin(g_rotationAngle);
      
      float x_rot = point3D[0] * cos_rot - point3D[2] * sin_rot;
      float y_rot = point3D[1];
      float z_rot = point3D[0] * sin_rot + point3D[2] * cos_rot;
      
      x2D = x_rot;
      y2D = y_rot;
    }

    void drawTrajectory() {
      if (g_pointCount == 0) return;
      
      int numPoints = (g_pointCount < MAX_POINTS) ? g_pointCount : MAX_POINTS;
      
      float screenX[LORENZ_MAX_POINTS];
      float screenY[LORENZ_MAX_POINTS];
      float minX = 999, maxX = -999, minY = 999, maxY = -999;
      
      for (int i = 0; i < numPoints; i++) {
        int idx = (g_pointCount < MAX_POINTS) ? i : (g_trajectoryIndex + i) % MAX_POINTS;
        project3DTo2D(g_trajectory[idx], screenX[i], screenY[i]);
        
        if (screenX[i] < minX) minX = screenX[i];
        if (screenX[i] > maxX) maxX = screenX[i];
        if (screenY[i] < minY) minY = screenY[i];
        if (screenY[i] > maxY) maxY = screenY[i];
      }
      
      float rangeX = maxX - minX;
      float rangeY = maxY - minY;
      
      if (rangeX < 0.1) rangeX = 0.1;
      if (rangeY < 0.1) rangeY = 0.1;
      
      float scaleX = 120 / rangeX;
      float scaleY = 120 / rangeY;
      float scale = (scaleX < scaleY) ? scaleX : scaleY;
      
      float centerX = (minX + maxX) / 2;
      float centerY = (minY + maxY) / 2;
      
      for (int i = 0; i < numPoints; i++) {
        screenX[i] = (screenX[i] - centerX) * scale + 100;
        screenY[i] = (screenY[i] - centerY) * scale + 100;
        
        if (screenX[i] < 5) screenX[i] = 5;
        if (screenX[i] > 195) screenX[i] = 195;
        if (screenY[i] < 5) screenY[i] = 5;
        if (screenY[i] > 195) screenY[i] = 195;
      }
      
      for (int i = 0; i < numPoints; i++) {
        int x = (int)screenX[i];
        int y = (int)screenY[i];
        
        if (x >= 0 && x < 200 && y >= 0 && y < 200) {
          for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
              if (x + dx >= 0 && x + dx < 200 && y + dy >= 0 && y + dy < 200) {
                display.drawPixel(x + dx, y + dy, GxEPD_BLACK);
              }
            }
          }
        }
        
        if (i < numPoints - 1) {
          int nextI = i + 1;
          drawLine((int)screenX[i], (int)screenY[i], (int)screenX[nextI], (int)screenY[nextI]);
        }
      }
      
      if (numPoints > 0) {
        int lastIdx = (g_pointCount < MAX_POINTS) ? numPoints - 1 : (g_trajectoryIndex - 1 + MAX_POINTS) % MAX_POINTS;
        int currentX = (int)screenX[lastIdx];
        int currentY = (int)screenY[lastIdx];
        
        if (currentX >= 0 && currentX < 200 && currentY >= 0 && currentY < 200) {
          for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
              if (currentX + dx >= 0 && currentX + dx < 200 && currentY + dy >= 0 && currentY + dy < 200) {
                display.drawPixel(currentX + dx, currentY + dy, GxEPD_BLACK);
              }
            }
          }
        }
      }
      
    }

    void drawLine(int x0, int y0, int x1, int y1) {
      int dx = abs(x1 - x0);
      int dy = abs(y1 - y0);
      int sx = (x0 < x1) ? 1 : -1;
      int sy = (y0 < y1) ? 1 : -1;
      int err = dx - dy;
      
      while (true) {
        display.drawPixel(x0, y0, GxEPD_BLACK);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
          err -= dy;
          x0 += sx;
        }
        if (e2 < dx) {
          err += dx;
          y0 += sy;
        }
      }
    }

    float calculateMoonPhase() {
      const float MOON_CYCLE_DAYS = 29.53058867;
      
      long utcSeconds = 0;
      
      int year = currentTime.Year + 1970;
      int month = currentTime.Month;
      int day = currentTime.Day;
      int hour = currentTime.Hour;
      int minute = currentTime.Minute;
      int second = currentTime.Second;
      
      long daysSince2000 = 0;
      
      for (int y = 2000; y < year; y++) {
        daysSince2000 += (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
      }
      
      int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29;
      }
      for (int m = 1; m < month; m++) {
        daysSince2000 += daysInMonth[m - 1];
      }
      
      daysSince2000 += day - 1;
      
      utcSeconds = daysSince2000 * 86400L;
      utcSeconds += hour * 3600L;
      utcSeconds += minute * 60L;
      utcSeconds += second;
      
      long timezoneOffsetSeconds = TIMEZONE_OFFSET_HOURS * 3600L;
      utcSeconds -= timezoneOffsetSeconds;
      
      const float REF_NEW_MOON_DAYS = 5.0 + (18.0 / 24.0) + (14.0 / 1440.0);
      const float REF_NEW_MOON_SECONDS = REF_NEW_MOON_DAYS * 86400.0;
      
      float secondsSinceNewMoon = (float)utcSeconds - REF_NEW_MOON_SECONDS;
      
      float daysSinceNewMoon = secondsSinceNewMoon / 86400.0;
      
      float daysInCycle = fmod(daysSinceNewMoon, MOON_CYCLE_DAYS);
      if (daysInCycle < 0) daysInCycle += MOON_CYCLE_DAYS;
      float phase = daysInCycle / MOON_CYCLE_DAYS;
      
      return phase;
    }

    void drawMoonPhase() {
      float phase = calculateMoonPhase();
      
      int moonRadius = 10;
      int moonX = 200 - moonRadius - 8;
      int moonY = moonRadius + 2;
      
      bool waxing = (phase < 0.5);
      float illumination = phase < 0.5 ? phase * 2.0 : (1.0 - phase) * 2.0;
      
      for (int x = -moonRadius; x <= moonRadius; x++) {
        for (int y = -moonRadius; y <= moonRadius; y++) {
          float dist = sqrt(x*x + y*y);
          int px = moonX + x;
          int py = moonY + y;
          
          if (px < 0 || px >= 200 || py < 0 || py >= 200) continue;
          
          if (dist > moonRadius - 0.5 && dist <= moonRadius + 0.5) {
            display.drawPixel(px, py, GxEPD_BLACK);
          }
          else if (dist < moonRadius - 1) {
            bool shouldFill = false;
            
            if (illumination > 0.95) {
              shouldFill = true;
            } else if (illumination < 0.05) {
              shouldFill = false;
            } else {
              float xPos = x / (float)moonRadius;
              
              if (waxing) {
                float threshold = -1.0 + 2.0 * illumination;
                shouldFill = (xPos >= threshold);
              } else {
                float threshold = 1.0 - 2.0 * illumination;
                shouldFill = (xPos <= threshold);
              }
            }
            
            if (shouldFill) {
              display.drawPixel(px, py, GxEPD_BLACK);
            }
          }
        }
      }
    }

    int calculateDayOfYear() {
      int year = currentTime.Year + 1970;
      int month = currentTime.Month;
      int day = currentTime.Day;
      
      int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29;
      }
      
      int dayOfYear = day;
      for (int m = 1; m < month; m++) {
        dayOfYear += daysInMonth[m - 1];
      }
      
      return dayOfYear;
    }

    void calculateSunriseSunset(float& sunriseHour, float& sunsetHour) {
      float latRad = LATITUDE * PI / 180.0;
      int dayOfYear = calculateDayOfYear();
      
      float declination = 23.45 * PI / 180.0 * sin(2.0 * PI * (284.0 + dayOfYear) / 365.0);
      
      float solarElevation = -0.83 * PI / 180.0;
      
      float cosHourAngle = (sin(solarElevation) - sin(latRad) * sin(declination)) / (cos(latRad) * cos(declination));
      
      if (cosHourAngle > 1.0) {
        sunriseHour = -1.0;
        sunsetHour = -1.0;
        return;
      }
      if (cosHourAngle < -1.0) {
        sunriseHour = 0.0;
        sunsetHour = 24.0;
        return;
      }
      
      float hourAngle = acos(cosHourAngle);
      
      float B = 2.0 * PI * (dayOfYear - 81) / 365.0;
      float equationOfTime = 9.87 * sin(2.0 * B) - 7.53 * cos(B) - 1.5 * sin(B);
      
      float longitudeTimeOffset = LONGITUDE / 15.0;
      float timeCorrection = longitudeTimeOffset - TIMEZONE_OFFSET_HOURS + (equationOfTime / 60.0);
      
      float solarNoon = 12.0 - timeCorrection;
      
      float sunriseSolar = solarNoon - (hourAngle * 12.0 / PI);
      float sunsetSolar = solarNoon + (hourAngle * 12.0 / PI);
      
      sunriseHour = sunriseSolar;
      sunsetHour = sunsetSolar;
      
      if (sunriseHour < 0) sunriseHour += 24.0;
      if (sunriseHour >= 24.0) sunriseHour -= 24.0;
      if (sunsetHour < 0) sunsetHour += 24.0;
      if (sunsetHour >= 24.0) sunsetHour -= 24.0;
    }

    void drawSunriseSunset() {
      float sunriseHour, sunsetHour;
      calculateSunriseSunset(sunriseHour, sunsetHour);
      
      if (sunriseHour < 0 || sunsetHour < 0) {
        return;
      }
      
      int sunriseH = (int)sunriseHour;
      int sunriseM = (int)((sunriseHour - sunriseH) * 60.0);
      int sunsetH = (int)sunsetHour;
      int sunsetM = (int)((sunsetHour - sunsetH) * 60.0);
      
      String sunriseStr = "";
      if (sunriseH < 10) sunriseStr += "0";
      sunriseStr += String(sunriseH);
      if (sunriseM < 10) sunriseStr += "0";
      sunriseStr += String(sunriseM);
      
      String sunsetStr = "";
      if (sunsetH < 10) sunsetStr += "0";
      sunsetStr += String(sunsetH);
      if (sunsetM < 10) sunsetStr += "0";
      sunsetStr += String(sunsetM);
      
      String displayStr = sunriseStr + "/" + sunsetStr;
      
      display.setFont(&FreeMonoBold9pt7b);
      
      int16_t x1, y1;
      uint16_t w, h;
      display.getTextBounds(displayStr, 0, 0, &x1, &y1, &w, &h);
      
      int centerX = (200 - w) / 2;
      display.setCursor(centerX, 200 - 15 + 5);
      display.print(displayStr);
    }
};


watchySettings settings;
WatchFace m(settings);

void setup() {
  m.init("");
}

void loop() {
}
