#include "revolution.h"
#include "const.h"
#include "fonts/LibertinusSerif_Regular_French_ASCII13pt7b.h"
#include "fonts/LibertinusSerif_Regular_Numbers42pt7b.h"
#include <Arduino.h>

// Store in RTC RAM, otherwise we lose information between different interrupts
RTC_DATA_ATTR Revolution::Mode mode;

void decreaseMode()
{
    switch (mode) {
    case Revolution::Mode::DigitalDate:
        mode = Revolution::Mode::Digital;
        break;
    case Revolution::Mode::Digital:
        mode = Revolution::Mode::AnalogDate;
        break;
    case Revolution::Mode::AnalogDate:
        mode = Revolution::Mode::Analog;
        break;
    case Revolution::Mode::Analog:
        mode = Revolution::Mode::DigitalDate;
        break;
    }
}

void increaseMode()
{
    switch (mode) {
    case Revolution::Mode::DigitalDate:
        mode = Revolution::Mode::Analog;
        break;
    case Revolution::Mode::Analog:
        mode = Revolution::Mode::AnalogDate;
        break;
    case Revolution::Mode::AnalogDate:
        mode = Revolution::Mode::Digital;
        break;
    case Revolution::Mode::Digital:
        mode = Revolution::Mode::DigitalDate;
        break;
    }
}

Revolution::Revolution(bool darkMode, bool yearRoman, float_t handWidth, FrenchRepublicanCalendar::Language dayNameLang)
    : calendar(dayNameLang), decimalTime(), darkMode(darkMode), yearRoman(yearRoman), handWidth(handWidth)
{
}

void Revolution::drawWatchFace()
{
    if (this->darkMode) {
        this->display.fillScreen(GxEPD_BLACK);
        this->display.setTextColor(GxEPD_WHITE);
    } else {
        this->display.fillScreen(GxEPD_WHITE);
        this->display.setTextColor(GxEPD_BLACK);
    }

#ifdef DEBUG
    Serial.printf("Mode: %d\n", mode);
#endif

    switch (mode) {
    case Mode::DigitalDate:
        this->drawDate();
    case Mode::Digital:
        this->drawDigitalTime();
        break;
    case Mode::AnalogDate:
        this->drawDate();
    case Mode::Analog:
        this->drawAnalogTime();
        break;
    default:
        break;
    }
}

void Revolution::drawDigitalTime()
{
    const unsigned int hours = this->decimalTime.getHours();
    const unsigned int minutes = this->decimalTime.getMinutes();
    const int yOffset = mode == Mode::DigitalDate ? -45 : 0;
    char time[5];

    time[0] = '0' + hours % 10;
    time[1] = ':';
    time[2] = '0' + minutes / 10 % 10;
    time[3] = '0' + minutes % 10;
    time[4] = '\0';

    this->display.setFont(&LibertinusSerif_Regular_Numbers42pt7b);
    this->display.setTextWrap(false);

    this->drawCenteredString(time, GxEPD2_154_D67::WIDTH / 2, GxEPD2_154_D67::HEIGHT / 2 + yOffset);
}

void drawHand(GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> &display, int16_t len, float angle, float_t handWidth,
              uint16_t handColor)
{
    static const int16_t xc = GxEPD2_154_D67::WIDTH / 2;
    static const int16_t yc = GxEPD2_154_D67::HEIGHT / 2;

    int16_t xe = floor(-len * sin(angle));
    int16_t ye = floor(-len * cos(angle));

    const int16_t xb = round(-ye / (len / handWidth));
    const int16_t yb = round(xe / (len / handWidth));

    xe += xc;
    ye += yc;

    display.fillTriangle(xc - xb, yc - yb, xc + xb, yc + yb, xe + xb, ye + yb, handColor);
    display.fillTriangle(xc - xb, yc - yb, xe + xb, ye + yb, xe - xb, ye - yb, handColor);
}

void Revolution::drawAnalogTime()
{
    unsigned int hours = this->decimalTime.getHours();
    unsigned int minutes = this->decimalTime.getMinutes();

    const uint16_t handColor = this->darkMode ? GxEPD_WHITE : GxEPD_BLACK;

    // Hours
    drawHand(display, 60, (hours + minutes / 100.0) / 10.0 * -TWO_PI, this->handWidth, handColor);
    // Minutes
    drawHand(display, 90, minutes / 100.0 * -TWO_PI, this->handWidth, handColor);

    static const int16_t innerTickLenght = GxEPD2_154_D67::WIDTH / 20;
    static const int16_t centerX = GxEPD2_154_D67::WIDTH / 2;
    static const int16_t centerY = GxEPD2_154_D67::HEIGHT / 2;
    static const int16_t innerTickRadius = centerX - innerTickLenght;
    static const int16_t outerTickRadius = centerX;

    // Draw the ticks on the sides of the screen
    for (unsigned int tickIndex = 0; tickIndex < 10; tickIndex++) {
        unsigned int offset;

        switch (tickIndex) {
        case 2:
        case 3:
        case 7:
        case 8:
            offset = innerTickLenght / 2;
            break;
        case 1:
        case 4:
        case 6:
        case 9:
            offset = innerTickLenght * 2 + innerTickLenght / 5;
            break;
        default:
            offset = 0;
            break;
        }

        const float_t tickRotation = tickIndex * TWO_PI / 10;
        const float_t tickSin = sin(tickRotation);
        const float_t tickCos = cos(tickRotation);
        const float_t innerX = tickSin * (innerTickRadius + offset);
        const float_t innerY = -tickCos * (innerTickRadius + offset);
        const float_t outerX = tickSin * (outerTickRadius + offset);
        const float_t outerY = -tickCos * (outerTickRadius + offset);

        this->display.drawLine(centerX + innerX, centerY + innerY, centerX + outerX, centerY + outerY, handColor);
    }
}

void Revolution::drawDate()
{
    static const uint16_t x = GxEPD2_154_D67::WIDTH / 2;

    this->display.setTextWrap(false);
    this->display.setFont(&LibertinusSerif_Regular_French_ASCII13pt7b);

    const char *dayOfWeek = this->calendar.getWeekDayName();
    int day = this->calendar.getDay();
    const char *month = this->calendar.getMonthName();
    const String year = this->yearRoman ? this->calendar.getRomanizedYear() : String(this->calendar.getYear());
    const char *dayOfYear = this->calendar.getYearDayName();
    char *date;

    if (this->calendar.sansculottides()) {
        asprintf(&date, "%s", month);
    } else {
        asprintf(&date, "%d %s", day, month);
    }

    static const uint16_t y_offset = 22;
    uint16_t base_y;

    switch (mode) {
    case Mode::DigitalDate:
        base_y = GxEPD2_154_D67::HEIGHT / 2 + 12;
        this->drawCenteredString(dayOfWeek, x, base_y);
        this->drawCenteredString(date, x, base_y + y_offset);
        this->drawCenteredString(year.c_str(), x, base_y + y_offset * 2);
        this->drawCenteredString(dayOfYear, x, base_y + y_offset * 3);
        break;
    case Mode::AnalogDate:
        base_y = GxEPD2_154_D67::HEIGHT / 2 - 70;
        this->drawCenteredString(dayOfWeek, x, base_y);
        this->drawCenteredString(date, x, base_y + y_offset);
        this->drawCenteredString(year.c_str(), x, base_y + y_offset * 2);
        base_y += 120;
        this->drawCenteredString(dayOfYear, x, base_y + y_offset);
        break;
    default:
        break;
    }

    free(date);
}

void Revolution::drawCenteredString(const char *str, const int x, const int y)
{
    int16_t x1, y1;
    uint16_t w, h;

    this->display.getTextBounds(str, x, y, &x1, &y1, &w, &h);
    this->display.setCursor(x - w / 2, y + h / 2);
    this->display.print(str);
}

// Reimplemented from Watchy to use ALARM1 instead of ALARM2
void Revolution::init(String datetime)
{
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    Wire.begin(SDA, SCL);

    switch (wakeup_reason) {
#ifdef ESP_RTC
    case ESP_SLEEP_WAKEUP_TIMER: // ESP Internal RTC
        if (guiState == WATCHFACE_STATE) {
            this->RTC.read(this->currentTime);
            this->currentTime.Minute++;
            tmElements_t tm;
            tm.Month = this->currentTime.Month;
            tm.Day = this->currentTime.Day;
            tm.Year = this->currentTime.Year;
            tm.Hour = this->currentTime.Hour;
            tm.Minute = this->currentTime.Minute;
            tm.Second = 0;
            time_t t = makeTime(tm);
            this->RTC.set(t);
            this->RTC.read(this->currentTime);
            this->showWatchFace(true); // Partial updates on tick
        }
        break;
#endif
    case ESP_SLEEP_WAKEUP_EXT0:   // RTC Alarm
        this->RTC.alarm(ALARM_1); // Reset the alarm flag in the RTC
        if (guiState == WATCHFACE_STATE) {
            // Resets the alarm flag in the RTC
            this->resetAlarm();
            // Partial updates on tick
            this->showWatchFace(true);
        }
        break;
    case ESP_SLEEP_WAKEUP_EXT1: // Button press
        this->handleButtonPress();
        break;
    default: // Reset
#ifndef ESP_RTC
        this->_rtcConfig(datetime);
#endif
        this->calendar.update(makeTime(this->currentTime));
        this->decimalTime.update(this->currentTime);
        this->RTC.setAlarm(ALM1_MATCH_MINUTES, this->decimalTime.getNextAlarmWakeSeconds(),
                           this->decimalTime.getNextAlarmWakeMinutes(), 0, 0);

        // Skip the BMA423 configuration since we won't use it
        this->showWatchFace(false); // Full update on reset
        break;
    }

    deepSleep();
}

void Revolution::resetAlarm()
{
    this->RTC.alarm(ALARM_1);
    this->RTC.read(this->currentTime);
    this->calendar.update(makeTime(this->currentTime));
    this->decimalTime.update(this->currentTime);
    this->RTC.setAlarm(ALM1_MATCH_MINUTES, this->decimalTime.getNextAlarmWakeSeconds(),
                       this->decimalTime.getNextAlarmWakeMinutes(), 0, 0);
}

// Reimplemented from Watchy to use ALARM1 instead of ALARM2 and to switch between modes
void Revolution::handleButtonPress()
{
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if (wakeupBit & MENU_BTN_MASK) {
        // Menu button
        if (guiState == WATCHFACE_STATE) {
            // Enter menu state if coming from watch face
            this->showMenu(menuIndex, false);
        } else if (guiState == MAIN_MENU_STATE) {
            // If already in menu, then select menu item
            switch (menuIndex) {
            case 0:
                this->showBattery();
                break;
            case 1:
                this->showBuzz();
                break;
            case 2:
                this->showAccelerometer();
                break;
            case 3:
                this->setTime();
                break;
            case 4:
                this->setupWifi();
                break;
            case 5:
                this->showUpdateFW();
                break;
            default:
                break;
            }
        } else if (guiState == FW_UPDATE_STATE) {
            this->updateFWBegin();
        }
    } else if (wakeupBit & BACK_BTN_MASK) {
        // Back button
        if (guiState == MAIN_MENU_STATE) {
            // Exit to watch face if already in menu
            // Reset the alarm flag in the RTC
            this->resetAlarm();
            this->showWatchFace(false);
        } else if (guiState == APP_STATE) {
            // Exit to menu if already in app~
            this->showMenu(menuIndex, false);
        } else if (guiState == FW_UPDATE_STATE) {
            // Exit to menu if already in app
            this->showMenu(menuIndex, false);
        }
    } else if (wakeupBit & UP_BTN_MASK) {
        // Up button
        if (guiState == WATCHFACE_STATE) {
            increaseMode();

#ifdef DEBUG
    Serial.printf("Switching to watchface mode %d\n", mode);
#endif

            // Resets the alarm flag in the RTC
            this->resetAlarm();
            this->showWatchFace(false);
        } else if (guiState == MAIN_MENU_STATE) {
            // Decrement menu index
            menuIndex--;
            if (menuIndex < 0) {
                menuIndex = MENU_LENGTH - 1;
            }
            this->showMenu(menuIndex, true);
        }
    } else if (wakeupBit & DOWN_BTN_MASK) {
        // Down Button
        if (guiState == WATCHFACE_STATE) {
            decreaseMode();

#ifdef DEBUG
    Serial.printf("Switching to watchface mode %d\n", mode);
#endif

            // Resets the alarm flag in the RTC
            this->resetAlarm();
            this->showWatchFace(false);
        } else if (guiState == MAIN_MENU_STATE) {
            // Increment menu index
            menuIndex++;
            if (menuIndex > MENU_LENGTH - 1) {
                menuIndex = 0;
            }
            this->showMenu(menuIndex, true);
        }
    }

    /***************** Fast menu *****************/
    bool timeout = false;
    long lastTimeout = millis();

    pinMode(MENU_BTN_PIN, INPUT);
    pinMode(BACK_BTN_PIN, INPUT);
    pinMode(UP_BTN_PIN, INPUT);
    pinMode(DOWN_BTN_PIN, INPUT);

    while (!timeout) {
        if (millis() - lastTimeout > 5000) {
            timeout = true;
        } else {
            if (digitalRead(MENU_BTN_PIN) == 1) {
                lastTimeout = millis();
                if (guiState == MAIN_MENU_STATE) {
                    // If already in menu, then select menu item
                    switch (menuIndex) {
                    case 0:
                        this->showBattery();
                        break;
                    case 1:
                        this->showBuzz();
                        break;
                    case 2:
                        this->showAccelerometer();
                        break;
                    case 3:
                        this->setTime();
                        break;
                    case 4:
                        this->setupWifi();
                        break;
                    case 5:
                        this->showUpdateFW();
                        break;
                    default:
                        break;
                    }
                } else if (guiState == FW_UPDATE_STATE) {
                    this->updateFWBegin();
                }
            } else if (digitalRead(BACK_BTN_PIN) == 1) {
                lastTimeout = millis();
                if (guiState == MAIN_MENU_STATE) {
                    // Exit to watch face if already in menu
                    // Resets the alarm flag in the RTC
                    this->resetAlarm();
                    this->showWatchFace(false);

                    // Leave loop
                    break;
                } else if (guiState == APP_STATE) {
                    // Exit to menu if already in app
                    this->showMenu(menuIndex, false);
                } else if (guiState == FW_UPDATE_STATE) {
                    // Exit to menu if already in app
                    this->showMenu(menuIndex, false);
                }
            } else if (digitalRead(UP_BTN_PIN) == 1) {
                lastTimeout = millis();
                if (guiState == WATCHFACE_STATE) {
                    increaseMode();

        #ifdef DEBUG
            Serial.printf("Switching to watchface mode %d\n", mode);
        #endif

                    // Resets the alarm flag in the RTC
                    this->resetAlarm();
                    this->showWatchFace(false);

                    // Leave loop
                    break;
                } else if (guiState == MAIN_MENU_STATE) {
                    // Decrement menu index
                    menuIndex--;
                    if (menuIndex < 0) {
                        menuIndex = MENU_LENGTH - 1;
                    }
                    this->showFastMenu(menuIndex);
                }
            } else if (digitalRead(DOWN_BTN_PIN) == 1) {
                lastTimeout = millis();
                if (guiState == WATCHFACE_STATE) {
                    decreaseMode();

        #ifdef DEBUG
            Serial.printf("Switching to watchface mode %d\n", mode);
        #endif

                    // Resets the alarm flag in the RTC
                    this->resetAlarm();
                    this->showWatchFace(false);

                    // Leave loop
                    break;
                } else if (guiState == MAIN_MENU_STATE) {
                    // Increment menu index
                    menuIndex++;
                    if (menuIndex > MENU_LENGTH - 1) {
                        menuIndex = 0;
                    }
                    this->showFastMenu(menuIndex);
                }
            }
        }
    }

    display.hibernate();
}

// Reimplemented from Watchy to use ALARM1 instead of ALARM2
void Revolution::_rtcConfig(String datetime)
{
    // https://github.com/JChristensen/DS3232RTC
    this->RTC.squareWave(SQWAVE_NONE); // disable square wave output
    // Alarm wakes up Watchy every second
    this->RTC.setAlarm(ALM1_EVERY_SECOND, 0, 0, 0, 0);
    // Enable alarm interrupt
    this->RTC.alarmInterrupt(ALARM_1, true);

    this->RTC.read(this->currentTime);
}
