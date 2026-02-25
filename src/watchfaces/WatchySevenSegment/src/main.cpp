#if defined(WATCHY_WRAPPER_INCLUDE) || defined(WATCHY_STANDALONE_WATCHFACE)
#include <Arduino.h>
#include "../../../watchy/Watchy.h"
#include "../../../sdk/UiSDK.h"

#include "FourDigitDisplay.h"
#include "qrcode.h"
#include "Code128Barcode.h"
#include "QRBarcode.h"
#include "AztecBarcode.h"

#define W_12_HOUR_MODE    0
#define W_24_HOUR_MODE    1
#define W_DATE_MODE       2
#define W_SECONDS_MODE    3
#define W_STOPWATCH_MODE  4
#define W_VOLTAGE_MODE    5

#define W_ANALOG_MODE         10
#define W_ANALOG_SECONDS_MODE 1100 // Disabled.... by changing the number
#define W_QR_CODE_MODE        11

#define W_BARCODE_MODE  20 


#define QR_CODE_VERSION 4
#define QR_CODE_PIXEL_SIZE 5 // 6 pixels would work for this version 4 (33 x 33 pixels) but 5 gives a larger white border
#define QR_CODE_URL_TO_DISPLAY "https://github.com/qcope/WatchySevenSegment"

#define SMALL_TIME_BORDER 3
#define LARGE_TICK_ANGLE 2 
#define ANALOGUE_RADIUS 90
#define HOUR_HAND_LENGTH 55
#define HOUR_HAND_WIDTH 20
#define MINUTE_HAND_LENGTH 75
#define MINUTE_HAND_WIDTH 20
#define SECOND_HAND_LENGTH 80
#define SECOND_HAND_WIDTH 4
#define CENTRE_CIRCLE_RADIUS 2
#define HAND_POINTS 9 // number of coordinates to define the hand
#define HAND_BORDER 6
#define FAT_END 3 // Make the hand bigger at the end!

#define SECONDS_DISPLAY_TIME 5 // Max number of seconds to display the seconds for

#define BARCODE_ENCODING_CODE128 0
#define BARCODE_ENCODING_AZTEC 1
#define BARCODE_ENCODING_QR 2

#define BATTERY_HIGH_VOLTAGE 3.9
#define BATTERY_LOW_VOLTAGE 3.2

RTC_DATA_ATTR int watchMode = W_VOLTAGE_MODE;
RTC_DATA_ATTR bool qrTimeMode = true; // first button press... actually negates this... so you see URL rather that time first thing
RTC_DATA_ATTR int runMinutes = 0; // Number of minutes we have run for
RTC_DATA_ATTR time_t lastSyncTime = 0;
RTC_DATA_ATTR time_t firstSyncTime = 0;
RTC_DATA_ATTR int drift = 0; // Seconds we have drifted between syncs
RTC_DATA_ATTR bool manualSync = true;
RTC_DATA_ATTR int validSyncs = 0;
RTC_DATA_ATTR int fiddleSeconds = 0;      // Number of seconds to add or remove every hour
RTC_DATA_ATTR bool fiddleArmed = false;   // Flag to help ensure we only fiddle once per hour
RTC_DATA_ATTR time_t initialTime = 0;
RTC_DATA_ATTR int runMins = 0;
RTC_DATA_ATTR bool syncRequired = false;
RTC_DATA_ATTR time_t secondsDisplayedSince = 0;

RTC_DATA_ATTR unsigned long swSeconds = 0;
RTC_DATA_ATTR unsigned long swMinutes = 0;
RTC_DATA_ATTR bool swRunning = false;
RTC_DATA_ATTR time_t swStartTime = 0;

class LoyaltyCard {
  private:
    String name;
    String number;
    int encoding;
  public:
    LoyaltyCard(String name, int encoding, String number) {
      this->name = name;
      this->encoding = encoding;
      this->number = number;
    }
    String getName() {
      return name;
    }
    String getNumber() {
      return number;
    }
    int getEncoding() {
      return encoding;
    }
};

#define LOYALTY_CARD_COUNT 8

LoyaltyCard cards[LOYALTY_CARD_COUNT] = {
  // Tesco Clubcard translation:
  // Printed:                     634004027442760123
  // Encoded:                       9794027442760123
  // Printed:                     634004024032215123
  // Encoded:                       9794024032215123
  LoyaltyCard("Tesco",              BARCODE_ENCODING_AZTEC,     "9794024032215123"),
  LoyaltyCard("Waitrose",           BARCODE_ENCODING_CODE128,   "9210170713522123"),
  LoyaltyCard("Nectar",             BARCODE_ENCODING_CODE128,   "29940456381123"),
  LoyaltyCard("BP",                 BARCODE_ENCODING_CODE128,   "24900001398123"),
  LoyaltyCard("Marks and Spencer",  BARCODE_ENCODING_CODE128,   "982601333180374123"),
  LoyaltyCard("Bucks Library",      BARCODE_ENCODING_CODE128,   "05100104123"),
  LoyaltyCard("Coffee#1",           BARCODE_ENCODING_QR,        "40400015776408123"),
  LoyaltyCard("Parkrun",            BARCODE_ENCODING_CODE128,   "A10709123")
};



RTC_DATA_ATTR int currentCard = 0;
int cardCount = LOYALTY_CARD_COUNT;

class AnaloguePointer {
  private:
    int xCentre, yCentre, length, width;
    int xPoint, yPoint;
    int xPoints[HAND_POINTS], yPoints[HAND_POINTS];
    GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display;
    bool drawCentreWhite = true;
    uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  public:
    AnaloguePointer(int xCentre,
                    int yCentre,
                    int length,
                    int width,
                    GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display,
                    uint16_t fgColor,
                    uint16_t bgColor) {
      this->xCentre = xCentre;
      this->yCentre = yCentre;
      this->length = length;
      this->width = width;
      this->display = display;
      this->fgColor = fgColor;
      this->bgColor = bgColor;
      if (width < 10) {
        drawCentreWhite = false;
      }

      // lets imagine the pointer vertical and pointing up
      if (width >= 10) {
        xPoints[0] = xCentre - FAT_END - width / 2;
        xPoints[1] = xCentre + FAT_END + width / 2;
      } else {
        xPoints[0] = xCentre - width / 2;
        xPoints[1] = xCentre + width / 2;
      }
      xPoints[2] = xCentre + width / 2;
      xPoints[3] = xCentre - width / 2;

      yPoints[0] = yCentre - length;
      yPoints[1] = yCentre - length;
      yPoints[2] = yCentre;
      yPoints[3] = yCentre;

      // We are going to have a tip at the end of the pointer
      xPoints[4] = xCentre;
      yPoints[4] = yCentre - length - width / 2; 

      // Let's have a centre rectangle in the hand
      xPoints[5] = xCentre - width / 2 + HAND_BORDER;
      xPoints[6] = xCentre + width / 2 - HAND_BORDER;
      xPoints[7] = xCentre + width / 2 - HAND_BORDER;
      xPoints[8] = xCentre - width / 2 + HAND_BORDER;

      yPoints[5] = yCentre - length + HAND_BORDER;
      yPoints[6] = yCentre - length + HAND_BORDER;
      yPoints[7] = yCentre - length / 3;
      yPoints[8] = yCentre - length / 3;
      
    }
    void rotate(int angle) {
      if ( (angle <= 0) || (angle >= 360) ) {
        return;
      }

      for (int i=0; i < HAND_POINTS; i++) {
        int xNew = xCentre + (xPoints[i] - xCentre) * cos(angle * PI / 180) - (yPoints[i] - yCentre) * sin(angle * PI / 180);
        int yNew = yCentre + (xPoints[i] - xCentre) * sin(angle * PI / 180) + (yPoints[i] - yCentre) * cos(angle * PI / 180);
        xPoints[i] = xNew;
        yPoints[i] = yNew;
      }
      
    }
    void draw() {
      // draw a black hand lets have a triangle at the tip
      display->fillTriangle(xPoints[0], yPoints[0], xPoints[1], yPoints[1], xPoints[2], yPoints[2], fgColor);
      display->fillTriangle(xPoints[3], yPoints[3], xPoints[2], yPoints[2], xPoints[0], yPoints[0], fgColor);
      // display->fillCircle(xPoints[4], yPoints[4], width / 2, fgColor);
      // Now the tip
      display->fillTriangle(xPoints[0], yPoints[0], xPoints[1], yPoints[1], xPoints[4], yPoints[4], fgColor);
      display->fillCircle(xCentre, yCentre, width / 2, fgColor);
      // draw a white inner part to the hand
      // display->fillCircle(xPoints[4], yPoints[4], (width / 2) - HAND_BORDER, fgColor);
      // display->fillCircle(xCentre, yCentre, width / 2, fgColor);
      if (drawCentreWhite) {
        display->fillTriangle(xPoints[5], yPoints[5], xPoints[6], yPoints[6], xPoints[7], yPoints[7], bgColor);
        display->fillTriangle(xPoints[8], yPoints[8], xPoints[7], yPoints[7], xPoints[5], yPoints[5], bgColor);
      }
    }
};


class MyFirstWatchFace : public Watchy{ //inherit and extend Watchy class
    public:
        MyFirstWatchFace(const watchySettings& s) : Watchy(s) {
          if (initialTime == 0) {
            initialTime = time(NULL);
            runMins = 0;
          }
          else {
            runMins = (time(NULL) - initialTime) / 60;
          }
        }

  uint16_t fgColor_ = UiSDK::getWatchfaceFg(BASE_POLARITY);
  uint16_t bgColor_ = UiSDK::getWatchfaceBg(BASE_POLARITY);

void shortSleep() { // like deep sleep but only sleep until the next second
  UiSDK::hibernate(display);
  RTC.clearAlarm();        // resets the alarm flag in the RTC
  #ifdef ARDUINO_ESP32S3_DEV
  esp_sleep_enable_ext0_wakeup((gpio_num_t)USB_DET_PIN, USB_PLUGGED_IN ? LOW : HIGH); //// enable deep sleep wake on USB plug in/out
  rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_LOW); // enable deep sleep wake on button press
  rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);

  rtc_clk_32k_enable(true);
  //rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long int microsecToNextSec =  1000000 - tv.tv_usec;

  esp_sleep_enable_timer_wakeup(microsecToNextSec);
  #else
  // Set GPIOs 0-39 to input to avoid power leaking out
  const uint64_t ignore = 0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
  for (int i = 0; i < GPIO_NUM_MAX; i++) {
    if ((ignore >> i) & 0b1)
      continue;
    pinMode(i, INPUT);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN,
                               0); // enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_HIGH); // enable deep sleep wake on button press
  #endif

  if (microsecToNextSec > 0) {
    esp_deep_sleep_start();
  }

}

        bool syncNTP() { 
          // for now let's call the parent class syncNTP
          time_t now,secondSync,newNow;

          now = time(NULL);
          bool result = Watchy::syncNTP();
          if (result) {
            lastSyncTime = time(NULL);
            validSyncs++;
            drift = lastSyncTime - now;
            if (validSyncs == 2) { // this is the first sync... so we can calculate the drift
              firstSyncTime = lastSyncTime;
            }
            if (validSyncs == 3) { // this is the second sync... so we can calculate the drift
              secondSync = lastSyncTime;
              int runMinutes = (secondSync - firstSyncTime) / 60;
              if (runMinutes != 0) {
                int runHours = runMinutes / 60;
                fiddleSeconds = (runHours != 0) ? drift / runHours : 0;
              } else {
                fiddleSeconds = 0;
              }
            }
          }
          return result;
        }

        void drawWatchFace(){ //override this method to customize how the watch face looks

          UiSDK::initScreen(display);
          fgColor_ = UiSDK::getWatchfaceFg(BASE_POLARITY);
          bgColor_ = UiSDK::getWatchfaceBg(BASE_POLARITY);

          // Step counter managed by core Watchy
          if (syncRequired  || (currentTime.Hour == 0 && currentTime.Minute == 0)){
            if (connectWiFi()) {
             syncNTP();
            }
            syncRequired = false; // let's reset flag, we may have done the sync... of course it could have failed. 
          }
          // debug stuff over time sync
          runMinutes++;
          if ((fiddleSeconds != 0) &&  (currentTime.Minute == 58)) {
            fiddleArmed = true;
          }
          // Ok... if we have got some idea of how poor the RTC is... let's fiddle with the time
          if ( (validSyncs > 0) && (fiddleSeconds != 0) && fiddleArmed && (currentTime.Minute==59)) { // All conditions met... let's fiddle
            time_t now = time(NULL);
            now += fiddleSeconds;
            struct tm timeInfo;
            struct timeval tv;
            localtime_r(&now, &timeInfo);
            tv.tv_sec = mktime(&timeInfo);
            tv.tv_usec = 0;
            settimeofday(&tv, NULL); 
            fiddleArmed = false; // make sure we only fiddle once per hour
          }
          // Let's handle the stop watch
          /*
          if (swRunning) {
            swMinutes = 0;
            swSeconds = time(NULL) - swStartTime;
            if (swSeconds >= 60) {
              swMinutes = swSeconds / 60;
              swSeconds = swSeconds % 60;
              swMinutes = swMinutes % 100;
            }
          }
          */

          // Now if we are going to display a QR code.... let's do that first
          if (watchMode == W_QR_CODE_MODE) {
            char stringToCreateCodeFor[90];         // refer to QRCode library, README.md that explains the Data Capabalities of each
                                                    // of the QR code versions.  For example version 8 can handle 122 characters for Alphanumeric
            int errorCorrection=ECC_MEDIUM; // default... we may go higher... for a short string
            if (qrTimeMode) {
              time_t now = time(NULL);
              sprintf(stringToCreateCodeFor, 
              "T: %02d:%02d MIN %d DFT %d FID %d SYN %d NOW %ld FST %ld",
                                            currentTime.Hour, 
                                            currentTime.Minute, runMins,drift,fiddleSeconds,validSyncs,
                                            now,
                                            firstSyncTime
                                            );
              
            } else {
              sprintf(stringToCreateCodeFor, "%s",QR_CODE_URL_TO_DISPLAY);
            }
            QRCode qrcode;
            int qrcodeversion=QR_CODE_VERSION;
            
            uint8_t qrcodeBytes[qrcode_getBufferSize(qrcodeversion)];
            int pixelSize = QR_CODE_PIXEL_SIZE;
            qrcode_initText(&qrcode, qrcodeBytes, qrcodeversion, errorCorrection, stringToCreateCodeFor);
            int xoffset = (UiSDK::width(display) - qrcode.size * pixelSize) / 2;
            int yoffset = (UiSDK::height(display) - qrcode.size * pixelSize) / 2;

            for (uint8_t y = 0; y < qrcode.size; y++) {
                for (uint8_t x = 0; x < qrcode.size; x++) {
                    if(qrcode_getModule(&qrcode, x, y)){
                      UiSDK::fillRect(display, xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, fgColor_);
                    } else {
                      UiSDK::fillRect(display, xoffset + x * pixelSize, yoffset + y * pixelSize, pixelSize, pixelSize, bgColor_);
                    }
                }
            }
          }

          if ( (watchMode == W_12_HOUR_MODE) || (watchMode == W_24_HOUR_MODE) || (watchMode == W_DATE_MODE) || (watchMode == W_VOLTAGE_MODE) 
                || (watchMode == W_STOPWATCH_MODE) || (watchMode == W_SECONDS_MODE))
          {
            // Flags are 12H, 24H, DATE, SEC, AM, PM, SW, VOLT, RUN, LAP
            int xFlag[10]={30,30,UiSDK::width(display)/2,UiSDK::width(display)/2,UiSDK::width(display)-30,UiSDK::width(display)-30,30,30,UiSDK::width(display)/2,UiSDK::width(display)/2};
            int yFlag[10]={10,35,10,35,10,35,UiSDK::height(display)-11,UiSDK::height(display)-35,UiSDK::height(display)-11,UiSDK::height(display)-35};
            int xFlagWidth = 60;
            int yFlagHeight = 21;
            int radius = 5;
            int16_t x1,y1;
            uint16_t textWidth,textHeight;

            // lets draw the flags
            UiSDK::setFont(display, &FreeMonoBold9pt7b);
            UiSDK::setTextColor(display, fgColor_);

            if (watchMode == W_12_HOUR_MODE) {
              UiSDK::drawRoundRect(display, xFlag[0]-xFlagWidth/2,yFlag[0]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "12H",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[0]-textWidth/2,yFlag[0]+textHeight/2);
              UiSDK::print(display, "12H");
              if (currentTime.Hour < 12) {
                UiSDK::drawRoundRect(display, xFlag[4]-xFlagWidth/2,yFlag[4]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
                UiSDK::getTextBounds(display, "AM",0,0,&x1,&y1,&textWidth,&textHeight);
                UiSDK::setCursor(display, xFlag[4]-textWidth/2,yFlag[4]+textHeight/2);
                UiSDK::print(display, "AM");
              } else {
                UiSDK::drawRoundRect(display, xFlag[5]-xFlagWidth/2,yFlag[5]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
                UiSDK::getTextBounds(display, "PM",0,0,&x1,&y1,&textWidth,&textHeight);
                UiSDK::setCursor(display, xFlag[5]-textWidth/2,yFlag[5]+textHeight/2);
                UiSDK::print(display, "PM");
              }
            } 
            if (watchMode == W_24_HOUR_MODE) {
              UiSDK::drawRoundRect(display, xFlag[1]-xFlagWidth/2,yFlag[1]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "24H",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[1]-textWidth/2,yFlag[1]+textHeight/2);
              UiSDK::print(display, "24H");
            }
            if (watchMode == W_DATE_MODE) {
              UiSDK::drawRoundRect(display, xFlag[2]-xFlagWidth/2,yFlag[2]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "DATE",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[2]-textWidth/2,yFlag[2]+textHeight/2);
              UiSDK::print(display, "DATE");
            }
            if (watchMode == W_SECONDS_MODE) {
              UiSDK::drawRoundRect(display, xFlag[3]-xFlagWidth/2,yFlag[3]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "SEC",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[3]-textWidth/2,yFlag[3]+textHeight/2);
              UiSDK::print(display, "SEC");
            }
            if (watchMode == W_STOPWATCH_MODE) {
              UiSDK::drawRoundRect(display, xFlag[7]-xFlagWidth/2,yFlag[7]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "SW",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[7]-textWidth/2,yFlag[7]+textHeight/2);
              UiSDK::print(display, "SW");
            }
            if (watchMode == W_VOLTAGE_MODE) {
              UiSDK::drawRoundRect(display, xFlag[6]-xFlagWidth/2,yFlag[6]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
              UiSDK::getTextBounds(display, "VOLT",0,0,&x1,&y1,&textWidth,&textHeight);
              UiSDK::setCursor(display, xFlag[6]-textWidth/2,yFlag[6]+textHeight/2);
              UiSDK::print(display, "VOLT");
            }
            // Handle the Stop watch status
            if (swStartTime != 0) { // The stop watch has been initialised... let's display either run or lap
              if (swRunning) {
                UiSDK::drawRoundRect(display, xFlag[8]-xFlagWidth/2,yFlag[8]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
                UiSDK::getTextBounds(display, "RUN",0,0,&x1,&y1,&textWidth,&textHeight);
                UiSDK::setCursor(display, xFlag[8]-textWidth/2,yFlag[8]+textHeight/2);
                UiSDK::print(display, "RUN");
              } else {
                UiSDK::drawRoundRect(display, xFlag[9]-xFlagWidth/2,yFlag[9]-yFlagHeight/2,xFlagWidth,yFlagHeight,radius,fgColor_);
                UiSDK::getTextBounds(display, "LAP",0,0,&x1,&y1,&textWidth,&textHeight);
                UiSDK::setCursor(display, xFlag[9]-textWidth/2,yFlag[9]+textHeight/2);
                UiSDK::print(display, "LAP");
              }
            }

            // Now let's draw a battery symbol at the bottom right of the screen
            int batteryWidth = 35;
            int batteryHeight = 15;
            int batteryX = (UiSDK::width(display) * 3)/4; 
            int batteryY = UiSDK::height(display)- 20;
            int batteryRadius = 3;

            int terminalWidth = batteryWidth + 4; // it's going to be black, we just need it to stick out a bit on the righht
            int terminalHeight = 9;
            int terminalX = batteryX;
            int terminalY = batteryY + (batteryHeight / 2) - terminalHeight/2;
            int terminalRadius = 1;

            UiSDK::fillRoundRect(display, batteryX,batteryY,batteryWidth,batteryHeight,batteryRadius,fgColor_);
            UiSDK::drawRoundRect(display, terminalX,terminalY,terminalWidth,terminalHeight,terminalRadius,fgColor_);

            // Now let's draw the battery level, we'll consider above 3.9V as full, below 3.6V as empty
            // Bit of a crazy way.... but first let's draw a white rectangle inside the battery.... it being empty
            // then we'll draw a black rectangle.... representing how fill we are.. 

            UiSDK::fillRect(display, batteryX+3,batteryY+3,batteryWidth-6,batteryHeight-6,bgColor_);

            float batteryVoltage = getBatteryVoltage();
            if (batteryVoltage > BATTERY_HIGH_VOLTAGE) {
              batteryVoltage = BATTERY_HIGH_VOLTAGE;
            }
            if (batteryVoltage < BATTERY_LOW_VOLTAGE) {
              batteryVoltage = BATTERY_LOW_VOLTAGE;
            }
            
            int batteryLevel = ((batteryVoltage - BATTERY_LOW_VOLTAGE) * 100) / (BATTERY_HIGH_VOLTAGE - BATTERY_LOW_VOLTAGE);
            int batteryLevelWidth = (batteryWidth - 6) * batteryLevel / 100;
            int batteryLevelHeight = batteryHeight - 6;
            int batteryLevelX = batteryX + 3;
            int batteryLevelY = batteryY + 3;
            UiSDK::fillRect(display, batteryLevelX,batteryLevelY,batteryLevelWidth,batteryLevelHeight,fgColor_);
 
            // Now let's display the time
            FourDigitDisplay oneLine = FourDigitDisplay(0,UiSDK::height(display)/4,UiSDK::width(display),UiSDK::height(display)/2 );
            if (watchMode == W_24_HOUR_MODE) {
              oneLine.setTwentyFourHour(true); //set the time format to 24hr.... default is 12hr
            }
            if (watchMode == W_DATE_MODE) {
              oneLine.setInt(currentTime.Day); //set the date on the display
            } else if (watchMode == W_VOLTAGE_MODE) {
              oneLine.setInt(getBatteryVoltage()* 100); //set the time on the display
              oneLine.setDecimalPoint(true);
            } else if (watchMode == W_SECONDS_MODE) {
              if (secondsDisplayedSince != 0) {
                oneLine.setInt(currentTime.Second); //set the time on the display
              }
              else {
                oneLine.setColon(false);
              }
            } else if (watchMode == W_STOPWATCH_MODE) {
              if (swRunning) { // only if stopwatch is running.. calculate time to display
                swMinutes = 0;
                swSeconds = time(NULL) - swStartTime;
                if (swSeconds >= 60) {
                  swMinutes = swSeconds / 60;
                  swSeconds = swSeconds % 60;
                  swMinutes = swMinutes % 100;
                }
              }
              oneLine.setSWTime(swMinutes,swSeconds); 
            } else { // must be the regular 12 or 24 hour mode
              oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
            }
            oneLine.display(&display); //display the time on the screen
            if (watchMode==W_SECONDS_MODE) {
              oneLine.display(&display);
            }

            // OK... if we have now displayed the time.... but we'll do something mad.... if we are looking at seconds
            // we'll go into a loop, updating the seconds... and only break out.... after 10 secs
            /*
            if (watchMode == W_SECONDS_MODE) {
              unsigned long currentMillis, previousMillis, firstMillis;
              int displayedSecond;

              firstMillis = millis();
              currentMillis = firstMillis;
              previousMillis = currentMillis;
              displayedSecond = currentTime.Second;
              // first force the time to be displayed....
              UiSDK::displayUpdate(display, true);

              pinMode(DOWN_BTN_PIN, INPUT);
              while( digitalRead(DOWN_BTN_PIN) == HIGH ) {
                if (displayedSecond != currentTime.Second) {
                  oneLine.setInt(currentTime.Second); //set the time on the display
                  oneLine.display(&display); //display the time on the screen
                  UiSDK::displayUpdate(display, true);
                  displayedSecond = currentTime.Second;
                }
                currentMillis = millis();
                RTC.read(currentTime);
              }
              // OK... we are done! Let's move onto a different mode
              oneLine.clearDisplay();
              oneLine.display(&display); // wipe screen
              UiSDK::displayUpdate(display, true);
              watchMode++;
            }
            */
          }

          if ((watchMode == W_ANALOG_MODE) || (watchMode == W_ANALOG_SECONDS_MODE)) {
            int xCentre = UiSDK::width(display)/2;
            int yCentre = UiSDK::height(display)/2;
            // Let's draw ticks for each minute
            int tickOffset;
            for(int i=0; i<=45; i+=6) {
                tickOffset = 100 * tan(i * PI / 180); // Who on earth wants RADIANS !!!!
                UiSDK::drawLine(display, xCentre, yCentre, xCentre + tickOffset, 0, fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, UiSDK::width(display), yCentre - tickOffset, fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, UiSDK::width(display), yCentre + tickOffset, fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, xCentre + tickOffset, UiSDK::height(display), fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, xCentre - tickOffset, UiSDK::height(display), fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, 0, yCentre + tickOffset, fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, 0, yCentre - tickOffset, fgColor_);
                UiSDK::drawLine(display, xCentre, yCentre, xCentre - tickOffset, 0, fgColor_); 
            }
            // Now draw some triangles for bigger marks for 12,3,6,9 HOUR MARKS 
            tickOffset = 100 * tan(LARGE_TICK_ANGLE * PI / 180);
            UiSDK::fillTriangle(display, xCentre, yCentre,xCentre + tickOffset, 0, xCentre - tickOffset, 0, fgColor_);
            UiSDK::fillTriangle(display, xCentre, yCentre,xCentre + tickOffset, UiSDK::height(display), xCentre - tickOffset, UiSDK::height(display), fgColor_);
            UiSDK::fillTriangle(display, xCentre, yCentre,0, yCentre + tickOffset, 0, yCentre - tickOffset, fgColor_);
            UiSDK::fillTriangle(display, xCentre, yCentre,UiSDK::width(display), yCentre + tickOffset, UiSDK::width(display), yCentre - tickOffset, fgColor_);
            // Now draw the triangles for 5, 10, 20, 25, 35, 40, 50, 55 minute marks
            int tickOffset1 = 100 * tan( ( 30 - LARGE_TICK_ANGLE /2) * PI /180);
            int tickOffset2 = 100 * tan( ( 30 + LARGE_TICK_ANGLE /2) * PI /180);
            UiSDK::fillTriangle(display, xCentre,yCentre,xCentre + tickOffset1, 0, xCentre + tickOffset2, 0, fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,UiSDK::width(display), yCentre - tickOffset1, UiSDK::width(display), yCentre - tickOffset2, fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,UiSDK::width(display), yCentre + tickOffset1, UiSDK::width(display), yCentre + tickOffset2, fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,xCentre + tickOffset1, UiSDK::height(display), xCentre + tickOffset2, UiSDK::height(display), fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,xCentre - tickOffset1, UiSDK::height(display), xCentre - tickOffset2, UiSDK::height(display), fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,0, yCentre + tickOffset1, 0, yCentre + tickOffset2, fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,0, yCentre - tickOffset1, 0, yCentre - tickOffset2, fgColor_);
            UiSDK::fillTriangle(display, xCentre,yCentre,xCentre - tickOffset1, 0, xCentre - tickOffset2, 0, fgColor_);
            // Now white space in the middle
            UiSDK::fillCircle(display, xCentre, yCentre, ANALOGUE_RADIUS, bgColor_);

            // Now let's work on the hands

            AnaloguePointer hourHand =    AnaloguePointer(xCentre, yCentre, HOUR_HAND_LENGTH,   HOUR_HAND_WIDTH,   &display, fgColor_, bgColor_);
            AnaloguePointer minuteHand =  AnaloguePointer(xCentre, yCentre, MINUTE_HAND_LENGTH, MINUTE_HAND_WIDTH, &display, fgColor_, bgColor_);
            AnaloguePointer secondHand =  AnaloguePointer(xCentre, yCentre, SECOND_HAND_LENGTH, SECOND_HAND_WIDTH, &display, fgColor_, bgColor_);

            int hourAngle = (currentTime.Hour % 12) * 30 + currentTime.Minute / 2;
            int minuteAngle = (currentTime.Minute * 6) + currentTime.Second / 10;
            int secondAngle = currentTime.Second * 6;
            
            hourHand.rotate(hourAngle);
            minuteHand.rotate(minuteAngle);
            secondHand.rotate(secondAngle);

            hourHand.draw();
            minuteHand.draw();
            if (watchMode == W_ANALOG_SECONDS_MODE) {
              secondHand.draw();
            }
            UiSDK::fillCircle(display, xCentre, yCentre, CENTRE_CIRCLE_RADIUS, bgColor_); // draw the centre circle
           }


          if ( (watchMode == W_QR_CODE_MODE) && qrTimeMode ) {
            int xPos =  60, yPos = 85, width = 80, height = 30;
            UiSDK::fillRect(display, xPos,yPos,width,height,bgColor_);
            UiSDK::drawRect(display, xPos,yPos,width,height,fgColor_);
            FourDigitDisplay oneLine = FourDigitDisplay(xPos+SMALL_TIME_BORDER,yPos+SMALL_TIME_BORDER,width - 2 * SMALL_TIME_BORDER,height - 2 * SMALL_TIME_BORDER);
            oneLine.setTwentyFourHour(true); //set the time format to 24hr.... default is 12hr
            oneLine.setTime(currentTime.Hour, currentTime.Minute); //set the time on the display
            oneLine.display(&display); //display the time on the screen
          }

          if (watchMode == W_BARCODE_MODE) 
          {
            if (cards[currentCard].getEncoding() == BARCODE_ENCODING_AZTEC){
              
              AztecBarcode barcode = AztecBarcode(cards[currentCard].getNumber(),cards[currentCard].getName(),&display);
              barcode.draw();
              
            }
            if (cards[currentCard].getEncoding() == BARCODE_ENCODING_CODE128){
              Code128Barcode barcode = Code128Barcode(cards[currentCard].getNumber(),
                                                        cards[currentCard].getName(),
                                                        &display);
              barcode.draw();
            }
            if (cards[currentCard].getEncoding() == BARCODE_ENCODING_QR){
              QRBarcode barcode = QRBarcode(cards[currentCard].getNumber(),cards[currentCard].getName(),&display);
              barcode.draw();
            }

          }
          if (watchMode == W_STOPWATCH_MODE) {
            UiSDK::displayUpdate(display, true);
            shortSleep();
          }
          if ((watchMode == W_SECONDS_MODE) || (watchMode == W_ANALOG_SECONDS_MODE)) {
            /*
            if (secondsDisplayedSince > 0) {
              if ((time(NULL) - secondsDisplayedSince) > SECONDS_DISPLAY_TIME) { // Let's save the screen by timing this function out.
                secondsDisplayedSince = 0;
                // Let's move onto an appropriate mode
                if (watchMode == W_SECONDS_MODE) {
                  watchMode = W_12_HOUR_MODE;
                } else {
                  watchMode = W_ANALOG_MODE;
                }
              }
            }
            else {
              secondsDisplayedSince = time(NULL); // let's mark this time as the start of the seconds display
            }
            */
            // We are going to display the seconds until 1 second passed the next minute
            if (currentTime.Second==1){
              if (watchMode == W_SECONDS_MODE) {
                watchMode = W_12_HOUR_MODE;
              } else {
                watchMode = W_ANALOG_MODE;
              }
            }
            UiSDK::displayUpdate(display, true);
            secondsDisplayedSince = time(NULL);
            shortSleep();
          } 
          else{
            secondsDisplayedSince = 0; // We aren't displaying seconds... so let's reset the counter
          }
          /*
          if (watchMode == W_STOPWATCH_MODE) {
            shortSleep();
          }
          */  
        }

        

        void handleButtonPress()
        { 
          if (guiState != WATCHFACE_STATE) 
          {
            Watchy::handleButtonPress();
            return;
          } 
          // if we get here.... we are in watchface mode

          bool partialRefresh = true;

          uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
          if (wakeupBit & UP_BTN_MASK) 
          {
            if ((watchMode < W_ANALOG_MODE) || (watchMode > W_QR_CODE_MODE) || ( (watchMode == W_QR_CODE_MODE) && qrTimeMode) ) 
            {
              if (watchMode!=W_STOPWATCH_MODE) {
                watchMode = W_ANALOG_MODE;
                qrTimeMode = false;
                partialRefresh = false;
              }
              else { // we are in stopwatch mode
                if (!swRunning){
                  if (swStartTime <= 0) { // Not started clock... so let's start
                    swStartTime = time(NULL);
                  }
                  swRunning=true;
                  vibMotor(75,4);
                }
                else {
                  swRunning=false; 
                  vibMotor(150,4);
                }
              }
            }
            else 
            {
              if ((watchMode == W_QR_CODE_MODE) && (qrTimeMode == false)) 
              {
                qrTimeMode = true;
              }
              else
              {
                watchMode += 1;
              }
            }
          }
          if (wakeupBit & DOWN_BTN_MASK) 
          {
            if (watchMode <= W_VOLTAGE_MODE)
            { 
              watchMode += 1;
              qrTimeMode = true;
              if (watchMode > W_VOLTAGE_MODE) 
              {
                watchMode = W_12_HOUR_MODE;
              }
            }
            else
            {
              watchMode = W_12_HOUR_MODE;
              partialRefresh = false;
            }
          }
          if (wakeupBit & MENU_BTN_MASK) 
          {
            Watchy::handleButtonPress();
            return;
          }
          if (wakeupBit & BACK_BTN_MASK) 
          { 
            if (watchMode != W_BARCODE_MODE) 
            {
              if (watchMode==W_STOPWATCH_MODE) {
                  if (!swRunning){
                    swStartTime = 0;
                    swMinutes = 0;
                    swSeconds = 0;
                    
                    vibMotor(75,4);
                  }
              } else {
                watchMode = W_BARCODE_MODE;
                currentCard=0;
              }
            } 
            else 
            {
              currentCard++;
              if (currentCard >= cardCount) 
              {
                currentCard = 0;
              }
            }
          }
          RTC.read(currentTime);
          showWatchFace(partialRefresh);
        }
      
};

void WatchfaceEntrypoint_WatchySevenSegment(Watchy &watchy) {
  MyFirstWatchFace face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}

void showWatchFace_WatchySevenSegment(Watchy &watchy) {
  WatchfaceEntrypoint_WatchySevenSegment(watchy);
}

// This watchface is integrated into another firmware via a wrapper that
// includes this file for the class implementation. Avoid defining Arduino
// entrypoints unless explicitly building as a standalone watchface.
#ifdef WATCHY_STANDALONE_WATCHFACE

MyFirstWatchFace m(settings);

void setup()
{

  #ifdef DEBUG
  Serial.begin(115200); //initialize serial monitor
  while (!Serial); //wait for serial monitor to open

  // Make display look nice
  // watchMode=W_12_HOUR_MODE;
  // swStartTime = 1; // force the LAP flag to be displayed

  //  watchMode = W_ANALOG_MODE;

  // watchMode = W_BARCODE_MODE;
  // watchMode = W_STOPWATCH_MODE;
  // swRunning = true;
  // swStartTime = 1;
  watchMode = W_12_HOUR_MODE;

  delay(1000); //wait for everything else to start up


  Serial.println("Starting Watchy!"); //print to serial monitor
  delay(1000);
  #endif

  m.init(); 
}

void loop()
{  // don't think we'll get here
}

#endif // WATCHY_STANDALONE_WATCHFACE

#endif // WATCHY_WRAPPER_INCLUDE || WATCHY_STANDALONE_WATCHFACE

