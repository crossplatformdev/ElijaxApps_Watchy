#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "BadForEye.h"

static const unsigned char *numbers[10] = {numbers0, numbers1, numbers2, numbers3, numbers4, numbers5, numbers6, numbers7, numbers8, numbers9};

void showWatchFace_BadForEye(Watchy &watchy){
    UiSDK::initScreen(watchy.display);

    const uint16_t fg = UiSDK::getWatchfaceFg(UiSDK::getThemePolarity());
    const uint16_t bg = UiSDK::getWatchfaceBg(UiSDK::getThemePolarity());

    // Window/frame bitmap uses the theme FG.
    UiSDK::drawBitmap(watchy.display, 0, 0, window, DISPLAY_WIDTH, DISPLAY_HEIGHT, fg);

    // Digits are rendered as "cutouts" using the theme BG.
    const uint8_t hourTens = static_cast<uint8_t>(watchy.currentTime.Hour / 10);
    const uint8_t hourOnes = static_cast<uint8_t>(watchy.currentTime.Hour % 10);
    const uint8_t minTens = static_cast<uint8_t>(watchy.currentTime.Minute / 10);
    const uint8_t minOnes = static_cast<uint8_t>(watchy.currentTime.Minute % 10);

    // Hour
    UiSDK::drawBitmap(watchy.display, 50, 10, numbers[hourTens], 39, 80, bg);
    UiSDK::drawBitmap(watchy.display, 110, 10, numbers[hourOnes], 39, 80, bg);

    // Minute
    UiSDK::drawBitmap(watchy.display, 50, 110, numbers[minTens], 39, 80, bg);
    UiSDK::drawBitmap(watchy.display, 110, 110, numbers[minOnes], 39, 80, bg);
}
