#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/UiTemplates.h"

static String formatCountdown(uint32_t ms) {
    uint32_t totalSec = (ms + 999) / 1000; // round up so display does not hit 00 early
    uint32_t hours = totalSec / 3600;
    uint32_t mins = (totalSec / 60) % 60;
    uint32_t secs = totalSec % 60;
    char buf[12];
    if (hours > 0) {
        snprintf(buf, sizeof(buf), "%02u:%02u:%02u", static_cast<unsigned>(hours), static_cast<unsigned>(mins), static_cast<unsigned>(secs));
    } else {
        snprintf(buf, sizeof(buf), "%02u:%02u", static_cast<unsigned>(mins), static_cast<unsigned>(secs));
    }
    return String(buf);
}

static uint32_t sPresetSeconds = 300;
static uint32_t sRemainingMs = 300 * 1000UL;
static bool sRunning = false;
static bool sFinished = false;
static bool sBuzzing = false;
static uint32_t sBuzzStartMs = 0;
static uint32_t sBuzzNextPulseMs = 0;
static uint32_t sLastTick = 0;
static volatile uint32_t sNextDraw = 0;
static volatile bool sExit = false;

static void resetRemaining() {
    sRemainingMs = sPresetSeconds * 1000UL;
}

static void timerBack(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    sRunning = false;
    sBuzzing = false;
    sExit = true;
}

static void timerMenu(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    if (sFinished) {
        resetRemaining();
    } else {
        sRunning = !sRunning;
    }
    sNextDraw = 0;    
}

static void timerUp(Watchy *watchy) {
    UiTemplates::waitForAllButtonsReleased(50, 100);
    if (sPresetSeconds < 6UL * 3600UL) {
        sPresetSeconds += 60;
        sRemainingMs += 60UL * 1000UL;
    }
    sFinished = false;
    sBuzzing = false;
    sNextDraw = 0;
}

static void timerDown(Watchy *watchy) {
    if (sPresetSeconds > 30) {
        sPresetSeconds -= 30;
    } else if (sPresetSeconds > 5) {
        sPresetSeconds = 5;
    }

    if (sRemainingMs > 30UL * 1000UL) {
        sRemainingMs -= 30UL * 1000UL;
    } else if (sRemainingMs > 5UL * 1000UL) {
        sRemainingMs = 5UL * 1000UL;
    } else {
        sRemainingMs = 5UL * 1000UL;
    }
    sFinished = false;
    sBuzzing = false;
    sNextDraw = 0;
}

static UIControlsRowLayout sTimerControls[4] = {
        {"BACK", &Watchy::backPressed},
        {"UP", &Watchy::upPressed},
        {"", &Watchy::menuPressed}, // label set at render time
        {"DOWN", &Watchy::downPressed},
};

void Watchy::showTimer() {
    guiState = APP_STATE;

    UiTemplates::waitForAllButtonsReleased();
    setButtonHandlers(timerBack, timerUp, timerMenu, timerDown);

    sPresetSeconds = 300;
    sRemainingMs = sPresetSeconds * 1000UL;
    sRunning = false;
    sFinished = false;
    sBuzzing = false;
    sBuzzStartMs = 0;
    sBuzzNextPulseMs = 0;
    sLastTick = millis();
    sNextDraw = 0;
    sExit = false;

    while (true) {
        uint32_t nowMs = millis();

        UiSDK::renderControlsRow(*this, sTimerControls);
        if (sExit) {
            clearButtonHandlers();
            showMenu(menuIndex);
            return;
        }

        // Finished alert: vibrate in short pulses for up to 10 seconds.
        if (sBuzzing) {
            if (nowMs - sBuzzStartMs >= 10000UL) {
                sBuzzing = false;
            } else if (nowMs >= sBuzzNextPulseMs) {
                vibMotor(90, 1);
                sBuzzNextPulseMs = nowMs + 450UL;
            }
        }

        if (sRunning) {
            uint32_t delta = nowMs - sLastTick;
            sLastTick = nowMs;
            if (delta >= sRemainingMs) {
                sRemainingMs = 0;
                sRunning = false;
                sFinished = true;
                sBuzzing = true;
                sBuzzStartMs = nowMs;
                sBuzzNextPulseMs = 0;
            } else {
                sRemainingMs -= delta;
            }
        } else {
            sLastTick = nowMs;
        }

        if (nowMs >= sNextDraw) {
            sNextDraw = nowMs + 300;
            RTC.read(currentTime);

            char nowBuf[16];
            snprintf(nowBuf, sizeof(nowBuf), "%02d:%02d", currentTime.Hour, currentTime.Minute);

            display.setFullWindow();
            display.clearScreen(UiSDK::getWatchfaceBg(BASE_POLARITY));

            UITextSpec title{};
            title.x = 64;
            title.y = 36;
            title.font = &FreeMonoBold9pt7b;
            title.fillBackground = false;
            title.invert          = false;
            title.text = "Timer";

            UITextSpec nowText{};
            nowText.x = 10;
            nowText.y = 56;
            nowText.font = UiSDK::tinyMono6x8();
            nowText.fillBackground = false;
            nowText.invert          = false;
            nowText.text = String("Now ") + nowBuf;

            const bool useBigFont = (sRemainingMs < 3600UL * 1000UL);

            UITextSpec countdown{};
            countdown.x = 8;
            countdown.y = useBigFont ? 120 : 110;
            countdown.font = useBigFont ? static_cast<const GFXfont *>(&DSEG7_Classic_Bold_53) : UiSDK::defaultFont();
            countdown.fillBackground = false;
            countdown.invert          = false;
            countdown.text = formatCountdown(sRemainingMs);

            UITextSpec status{};
            status.x = 10;
            status.y = 150;
            status.font = &FreeMonoBold9pt7b;
            status.fillBackground = false;
            status.invert          = false;
            if (sFinished) {
                status.text = "Done";
            } else if (sRunning) {
                status.text = "Running";
            } else {
                status.text = "Paused";
            }

            UITextSpec texts[] = {title, nowText, countdown, status};

            const char *acceptLabel = sFinished ? "RESTART" : (sRunning ? "PAUSE" : "START");
            
            UIAppSpec app{};
            app.texts = texts;
            app.textCount = 4;
            
            sTimerControls[2].label = acceptLabel;
            app.controls[0] = sTimerControls[0];
            app.controls[1] = sTimerControls[1];
            app.controls[2] = sTimerControls[2];
            app.controls[3] = sTimerControls[3];

            UiSDK::renderApp(*this, app);
        }
    }
}
