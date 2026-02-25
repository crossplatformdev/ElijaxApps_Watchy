#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "beastie.h"

namespace {

void drawWDay(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setFont(watchy.display, &conso10pt7b);
    UiSDK::setTextColor(watchy.display, fg);

    int16_t  x1, y1;
    uint16_t w, h;
    String dayOfWeek = dayShortStr(watchy.currentTime.Wday);

    UiSDK::getTextBounds(watchy.display, String(dayOfWeek), 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(watchy.display, 151 - w/2, 67);
    UiSDK::println(watchy.display, String(dayOfWeek));
}

void drawDate(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setFont(watchy.display, &conso12pt7b);
    UiSDK::setTextColor(watchy.display, fg);

    int16_t  x1, y1;
    uint16_t w, h;
    String monthStr = String(watchy.currentTime.Month);
    String dayStr = String(watchy.currentTime.Day);

    monthStr = watchy.currentTime.Month < 10 ? "0" + monthStr : monthStr;
    dayStr = watchy.currentTime.Day < 10 ? "0" + dayStr : dayStr;
    String dateStr = dayStr + "/" + monthStr;

    UiSDK::getTextBounds(watchy.display, String(dateStr), 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(watchy.display, 151 - w/2, 88);
    UiSDK::println(watchy.display, String(dateStr));
}

void drawTime(Watchy &watchy) {
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setFont(watchy.display, &conso17pt7b);
    UiSDK::setTextColor(watchy.display, bg);
    UiSDK::setCursor(watchy.display, 111, 119);
    UiSDK::fillRoundRect(watchy.display, 111, 95, 85, 29, 4, fg);

    if (watchy.currentTime.Hour < 10) {
        UiSDK::print(watchy.display, "0");
    }

    UiSDK::print(watchy.display, watchy.currentTime.Hour);
    UiSDK::print(watchy.display, ":");

    if (watchy.currentTime.Minute < 10) {
        UiSDK::print(watchy.display, "0");
    }

    UiSDK::print(watchy.display, watchy.currentTime.Minute);
}

void drawSteps(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setFont(watchy.display, &conso11pt7b);
    UiSDK::setTextColor(watchy.display, fg);

    // Step counter managed by core Watchy

    int16_t  x1, y1;
    uint16_t w, h;
    uint32_t stepCount = sensor.getCounter();
    char stepStr[32];

    itoa(stepCount, stepStr, 10);
    int stepStrL = strlen(stepStr);
    memset(stepStr, '0', 5);
    itoa(stepCount, stepStr + max(5-stepStrL, 0), 10);

    UiSDK::getTextBounds(watchy.display, String(stepStr), 0, 0, &x1, &y1, &w, &h);
    UiSDK::setCursor(watchy.display, 152 - w/2, 143);
    UiSDK::println(watchy.display, String(stepStr));
}

void drawBattery(Watchy &watchy) {
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    float BATTV = watchy.getBatteryVoltage() - 3.60;
    int batt_w = constrain(((33.33 * BATTV) + 0.9), 0, 20);

    UiSDK::fillRoundRect(watchy.display, 138, 150, 30, 10, 5, fg);
    UiSDK::fillRoundRect(watchy.display, 140, 152, 26, 6, 4, bg);

    if (BATTV > 0) {
        if (batt_w % 2 != 0) {
            UiSDK::fillRoundRect(watchy.display, 153 - (batt_w/2)-1, 154, batt_w, 2, 3, fg);
        }

        UiSDK::fillRoundRect(watchy.display, 153 - batt_w/2, 154, batt_w, 2, 3, fg);
    }
}

void drawX(Watchy &watchy) {
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setFont(watchy.display, &conso11pt7b);
    UiSDK::setTextColor(watchy.display, fg);
    UiSDK::setCursor(watchy.display, 149, 50);
    UiSDK::print(watchy.display, "x");
}

} // namespace

void showWatchFace_Beastie(Watchy &watchy) {
    UiSDK::initScreen(watchy.display);
    const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
    const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

    UIImageSpec daemon{};
    daemon.bitmap = daemon_img;
    daemon.x = 0;
    daemon.y = 0;
    daemon.w = 200;
    daemon.h = 200;
    daemon.fromProgmem = true;
    daemon.fillBackground = false;
    UiSDK::renderImage(watchy.display, daemon);

    drawX(watchy);
    drawWDay(watchy);
    drawDate(watchy);
    drawTime(watchy);
    drawSteps(watchy);
    drawBattery(watchy);
  //drawTemperature();
}
