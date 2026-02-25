#include "revolution.h"
#include "const.h"
#include "../../sdk/UiSDK.h"
#include "fonts/LibertinusSerif_Regular_French_ASCII13pt7b.h"
#include "fonts/LibertinusSerif_Regular_Numbers42pt7b.h"
#include <Arduino.h>

// Store in RTC RAM, otherwise we lose information between different interrupts
static RTC_DATA_ATTR Revolution::Mode revolution_mode = Revolution::Mode::DigitalDate;

static void decreaseMode()
{
    switch (revolution_mode) {
    case Revolution::Mode::DigitalDate:
        revolution_mode = Revolution::Mode::Digital;
        break;
    case Revolution::Mode::Digital:
        revolution_mode = Revolution::Mode::AnalogDate;
        break;
    case Revolution::Mode::AnalogDate:
        revolution_mode = Revolution::Mode::Analog;
        break;
    case Revolution::Mode::Analog:
        revolution_mode = Revolution::Mode::DigitalDate;
        break;
    }
}

static void increaseMode()
{
    switch (revolution_mode) {
    case Revolution::Mode::DigitalDate:
        revolution_mode = Revolution::Mode::Analog;
        break;
    case Revolution::Mode::Analog:
        revolution_mode = Revolution::Mode::AnalogDate;
        break;
    case Revolution::Mode::AnalogDate:
        revolution_mode = Revolution::Mode::Digital;
        break;
    case Revolution::Mode::Digital:
        revolution_mode = Revolution::Mode::DigitalDate;
        break;
    }
}

Revolution::Revolution(bool yearRoman, float_t handWidth, FrenchRepublicanCalendar::Language dayNameLang)
    : calendar(dayNameLang), decimalTime(), yearRoman(yearRoman), handWidth(handWidth)
{
}

void Revolution::drawWatchFace()
{
    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setTextColor(this->display, fgColor);

#ifdef DEBUG
    Serial.printf("Mode: %d\n", revolution_mode);
#endif

    switch (revolution_mode) {
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
    const int yOffset = revolution_mode == Mode::DigitalDate ? -45 : 0;
    char time[5];

    time[0] = '0' + hours % 10;
    time[1] = ':';
    time[2] = '0' + minutes / 10 % 10;
    time[3] = '0' + minutes % 10;
    time[4] = '\0';

    UiSDK::setFont(this->display, &LibertinusSerif_Regular_Numbers42pt7b);
    UiSDK::setTextWrap(this->display, false);

    this->drawCenteredString(time, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2 + yOffset);
}

template <typename DisplayT>
static void drawHand(DisplayT &display, int16_t len, float angle, float_t handWidth, uint16_t handColor)
{
    static const int16_t xc = DISPLAY_WIDTH / 2;
    static const int16_t yc = DISPLAY_HEIGHT / 2;

    int16_t xe = floor(-len * sin(angle));
    int16_t ye = floor(-len * cos(angle));

    const int16_t xb = round(-ye / (len / handWidth));
    const int16_t yb = round(xe / (len / handWidth));

    xe += xc;
    ye += yc;

    UiSDK::fillTriangle(display, xc - xb, yc - yb, xc + xb, yc + yb, xe + xb, ye + yb, handColor);
    UiSDK::fillTriangle(display, xc - xb, yc - yb, xe + xb, ye + yb, xe - xb, ye - yb, handColor);
}

void Revolution::drawAnalogTime()
{
    unsigned int hours = this->decimalTime.getHours();
    unsigned int minutes = this->decimalTime.getMinutes();

    const uint16_t handColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    // Hours
    drawHand(display, 60, (hours + minutes / 100.0) / 10.0 * -TWO_PI, this->handWidth, handColor);
    // Minutes
    drawHand(display, 90, minutes / 100.0 * -TWO_PI, this->handWidth, handColor);

    static const int16_t innerTickLenght = DISPLAY_WIDTH / 20;
    static const int16_t centerX = DISPLAY_WIDTH / 2;
    static const int16_t centerY = DISPLAY_HEIGHT / 2;
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

        UiSDK::drawLine(this->display, centerX + innerX, centerY + innerY, centerX + outerX, centerY + outerY, handColor);
    }
}

void Revolution::drawDate()
{
    static const uint16_t x = DISPLAY_WIDTH / 2;

    UiSDK::setTextWrap(this->display, false);
    UiSDK::setFont(this->display, &LibertinusSerif_Regular_French_ASCII13pt7b);

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

    switch (revolution_mode) {
    case Mode::DigitalDate:
        base_y = DISPLAY_HEIGHT / 2 + 12;
        this->drawCenteredString(dayOfWeek, x, base_y);
        this->drawCenteredString(date, x, base_y + y_offset);
        this->drawCenteredString(year.c_str(), x, base_y + y_offset * 2);
        this->drawCenteredString(dayOfYear, x, base_y + y_offset * 3);
        break;
    case Mode::AnalogDate:
        base_y = DISPLAY_HEIGHT / 2 - 70;
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

    UiSDK::getTextBounds(this->display, str, x, y, &x1, &y1, &w, &h);
    UiSDK::setCursor(this->display, x - w / 2, y + h / 2);
    UiSDK::print(this->display, str);
}

void Revolution::init(String datetime)
{
    // This repo renders Revolution via a wrapper draw function.
    // Keep init() compilable but avoid DS3232RTC-specific alarm APIs.
    this->_rtcConfig(datetime);
    this->calendar.update(makeTime(this->currentTime));
    this->decimalTime.update(this->currentTime);
    this->showWatchFace(false);
    this->deepSleep();
}

void Revolution::resetAlarm()
{
    this->RTC.clearAlarm();
    this->RTC.read(this->currentTime);
    this->calendar.update(makeTime(this->currentTime));
    this->decimalTime.update(this->currentTime);
}

// Reimplemented from Watchy to use ALARM1 instead of ALARM2 and to switch between modes
void Revolution::handleButtonPress()
{
    // This watchface is rendered by the OS menu system in this repo.
    // Keep simple mode switching on UP/DOWN when on the watchface.
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
    if (guiState == WATCHFACE_STATE) {
        if (wakeupBit & UP_BTN_MASK) {
            increaseMode();
        } else if (wakeupBit & DOWN_BTN_MASK) {
            decreaseMode();
        } else {
            Watchy::handleButtonPress();
            return;
        }

        this->RTC.clearAlarm();
        this->RTC.read(this->currentTime);
        this->calendar.update(makeTime(this->currentTime));
        this->decimalTime.update(this->currentTime);
        this->showWatchFace(false);
        return;
    }

    Watchy::handleButtonPress();
}

// Reimplemented from Watchy to use ALARM1 instead of ALARM2
void Revolution::_rtcConfig(String datetime)
{
    this->RTC.init();
    if (datetime != "") {
        this->RTC.config(datetime);
    }
    this->RTC.clearAlarm();
    this->RTC.read(this->currentTime);
}

void showWatchFace_Revolution(Watchy &watchy)
{
    Revolution face(
        /*yearRoman=*/true,
        /*handWidth=*/2.0,
        FrenchRepublicanCalendar::Language::French);
    face.settings = watchy.settings;
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}
