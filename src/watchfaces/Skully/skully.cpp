#include "skully.h"
#include "../../sdk/UiSDK.h"

namespace {

void drawTime(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &GorgeousPixel30pt7b);
    UiSDK::setTextColor(watchy.display, fgColor);

    int16_t  xh1, yh1;
    uint16_t wh1, hh1;
    int16_t  xh2, yh2;
    uint16_t wh2, hh2;
    int16_t  xm1, ym1;
    uint16_t wm1, hm1;
    int16_t  xm2, ym2;
    uint16_t wm2, hm2;

    String h = String(watchy.currentTime.Hour);
    String m = String(watchy.currentTime.Minute);
    String h1 = String(h.substring(0,1));
    String h2 = String(h.substring(1,2));
    String m1 = String(m.substring(0,1));
    String m2 = String(m.substring(1,2));

    UiSDK::getTextBounds(watchy.display, String(h1), 0, 0, &xh1, &yh1, &wh1, &hh1);
    UiSDK::getTextBounds(watchy.display, String(h2), 0, 0, &xh2, &yh2, &wh2, &hh2);
    UiSDK::getTextBounds(watchy.display, String(m1), 0, 0, &xm1, &ym1, &wm1, &hm1);
    UiSDK::getTextBounds(watchy.display, String(m2), 0, 0, &xm2, &ym2, &wm2, &hm2);
    UiSDK::setCursor(watchy.display, 142 - wh1/2, 144);

    if (watchy.currentTime.Hour < 10) {
        UiSDK::print(watchy.display, "0");
        UiSDK::setCursor(watchy.display, 174 - wh1/2, 144);
        UiSDK::print(watchy.display, String(h1));
    } else {
        UiSDK::print(watchy.display, String(h1));
        UiSDK::setCursor(watchy.display, 174 - wh2/2, 144);
        UiSDK::print(watchy.display, String(h2));
    }

    UiSDK::setCursor(watchy.display, 142 - wm1/2, 192);

    if (watchy.currentTime.Minute < 10) {
        UiSDK::print(watchy.display, "0");
        UiSDK::setCursor(watchy.display, 174 - wm1/2, 192);
        UiSDK::print(watchy.display, String(m1));
    } else {
        UiSDK::print(watchy.display, String(m1));
        UiSDK::setCursor(watchy.display, 174 - wm2/2, 192);
        UiSDK::print(watchy.display, String(m2));
    }
}

void drawWDay(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &GorgeousPixel11pt7b);
    UiSDK::setTextColor(watchy.display, bgColor);

    int16_t  x1, y1;
    uint16_t w, h;
    String dayOfWeek = dayShortStr(watchy.currentTime.Wday);
    dayOfWeek.toUpperCase();

    UiSDK::getTextBounds(watchy.display, String(dayOfWeek), 0, 0, &x1, &y1, &w, &h);
    UiSDK::fillRect(watchy.display, 186 - w, 50, w + 8, h + 5, fgColor);
    UiSDK::setCursor(watchy.display, 189 - w, 68);
    UiSDK::println(watchy.display, String(dayOfWeek));
}

void drawDate(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &GorgeousPixel9pt7b);
    UiSDK::setTextColor(watchy.display, fgColor);
    UiSDK::setCursor(watchy.display, 4, 195);

    String monthStr = String(watchy.currentTime.Month);
    String dayStr = String(watchy.currentTime.Day);
    monthStr = watchy.currentTime.Month < 10 ? "0" + monthStr : monthStr;
    dayStr = watchy.currentTime.Day < 10 ? "0" + dayStr : dayStr;
    String dateStr = dayStr + "." + monthStr;
    UiSDK::print(watchy.display, String(dateStr));
}

void drawSteps(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &GorgeousPixel9pt7b);
    UiSDK::setTextColor(watchy.display, fgColor);
    int16_t  x1, y1;
    uint16_t w, h;

    uint32_t stepCount = sensor.getCounter();
    char stepStr[32];
    itoa(stepCount, stepStr, 10);
    int stepStrL = strlen(stepStr);
    memset(stepStr, '0', 5);
    itoa(stepCount, stepStr + max(5-stepStrL, 0), 10);

    UiSDK::getTextBounds(watchy.display, String(stepStr), 0, 0, &x1, &y1, &w, &h);
    UiSDK::drawRect(watchy.display, 186 - w, 71, w + 8, h + 8, fgColor);
    UiSDK::setCursor(watchy.display, 189 - w, 87);
    UiSDK::println(watchy.display, stepStr);
}

void drawBattery(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    float BATTV = watchy.getBatteryVoltage() - 3.60;
    int batt_w = constrain(((33.33 * BATTV) + 0.9), 0, 20);
    UiSDK::fillRect(watchy.display, 166, 5, 28, 12, fgColor);
    UiSDK::fillRect(watchy.display, 163, 9, 3, 4, fgColor);
    UiSDK::fillRect(watchy.display, 168, 7, 24, 8, bgColor);

    if (BATTV > 0) {
        UiSDK::fillRect(watchy.display, 190 - batt_w, 9, batt_w, 4, fgColor);
    }
}

void drawTemperature(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::setFont(watchy.display, &GorgeousPixel8pt7b);
    UiSDK::setTextColor(watchy.display, fgColor);
    UiSDK::setCursor(watchy.display, 4, 179);
    uint8_t temperatureRTC = watchy.RTC.temperature() / 4;
    if (watchy.settings.weatherUnit == "imperial") {
        temperatureRTC = temperatureRTC * (9/5) + 32;
    }

    if (temperatureRTC < 10) {
        UiSDK::print(watchy.display, "0");
    }

    UiSDK::print(watchy.display, temperatureRTC);

    if (watchy.settings.weatherUnit == "imperial") {
        UiSDK::print(watchy.display, "f");
    } else {
        UiSDK::print(watchy.display, "c");
    }
}

} // namespace

void showWatchFace_Skully(Watchy &watchy) {
    UiSDK::initScreen(watchy.display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UiSDK::drawBitmap(watchy.display, 0, 0, pirate, 200, 200, fgColor);
    drawTime(watchy);
    drawWDay(watchy);
    drawDate(watchy);
    drawSteps(watchy);
    drawBattery(watchy);
  //drawTemperature(watchy);

    for (uint8_t i=0; i<3; i++) {
        UiSDK::displayUpdate(watchy.display, true);
    }
}
