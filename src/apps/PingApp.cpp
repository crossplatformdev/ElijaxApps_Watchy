#include "../watchy/Watchy.h"
#include "../sdk/UiSDK.h"
#include "../sdk/Fonts.h"
#include "../sdk/UiTemplates.h"
#include "../sdk/NetUtils.h"
#include <ESP32Ping.h>
#include <WiFi.h>
#include <string.h>
#include <stdio.h>

// Some toolchains/libraries may define macros that collide with struct field
// names used by our UI specs.
#ifdef texts
#undef texts
#endif
#ifdef images
#undef images
#endif
#ifdef menus
#undef menus
#endif
#ifdef buttons
#undef buttons
#endif
#ifdef checkboxes
#undef checkboxes
#endif
#ifdef scrollTexts
#undef scrollTexts
#endif
#ifdef callbacks
#undef callbacks
#endif

#ifdef WIDTH
#undef WIDTH
#endif
#ifdef HEIGHT
#undef HEIGHT
#endif

// Ping application with digit-by-digit IP editor and history.
// Input model: apps register 4 handlers; the fixed controls row callbacks are:
//   BACK -> Watchy::backPressed
//   UP -> Watchy::upPressed
//   ACCEPT -> Watchy::menuPressed
//   DOWN -> Watchy::downPressed

static const char *kPingHistoryNs = "hist_ping";
static int8_t sPingHistorySelected = -1;
static const char *kExampleLabel = "example.com";
static const char *kExampleTarget = "example.com";

// Daemon state persistence
RTC_DATA_ATTR bool daemonActive = false;
RTC_DATA_ATTR char daemonIpStr[16] = {0};
RTC_DATA_ATTR uint32_t totalSent = 0;
RTC_DATA_ATTR uint32_t totalReceived = 0;
RTC_DATA_ATTR uint32_t totalTimeSum = 0;
RTC_DATA_ATTR uint16_t daemonMinTime = 0;
RTC_DATA_ATTR uint16_t daemonMaxTime = 0;

// Daemon worker thread + shared buffers
static TaskHandle_t daemonTaskHandle = nullptr;
static SemaphoreHandle_t daemonStatsMutex = nullptr;
static uint16_t daemonPktTimes[4] = {0, 0, 0, 0};
static bool daemonPktOk[4] = {false, false, false, false};
static uint8_t daemonPktIndex = 0;

enum class PingMode : uint8_t
{
    Edit,
    Confirm,
    ConfirmTarget,
    Result,
    StopConfirm
};

struct PingState
{
    PingMode mode = PingMode::Edit;
    bool daemonMode = false;
    bool exitRequested = false;
    bool needsRender = true;

    uint8_t digits[12] = {0};
    uint8_t pos = 0; // 0-11 digit positions, 12 = confirm

    char lastTargetStr[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
    char lastIpStr[16] = {0};
    bool resultBadIp = false;

    static constexpr uint8_t PING_COUNT = 4;
    uint8_t sentCount = PING_COUNT;
    uint8_t recvCount = 0;
    uint16_t pktTimes[PING_COUNT] = {0, 0, 0, 0};
    bool pktOk[PING_COUNT] = {false, false, false, false};
    uint16_t minTime = 0;
    uint16_t maxTime = 0;
    uint16_t avgTime = 0;

    bool backHeld = false;
    uint32_t backHoldStartMs = 0;
};

static PingState sPing;

static UIControlsRowLayout sPingControls[4] = {
    {"BACK", &Watchy::backPressed},
    {"UP", &Watchy::upPressed},
    {"ACCEPT", &Watchy::menuPressed},
    {"DOWN", &Watchy::downPressed},
};

static void resetDaemonStatsLocked()
{
    daemonPktIndex = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        daemonPktTimes[i] = 0;
        daemonPktOk[i] = false;
    }
    totalSent = 0;
    totalReceived = 0;
    totalTimeSum = 0;
    daemonMinTime = 0;
    daemonMaxTime = 0;
}

static void copyDaemonStats(uint16_t outTimes[4],
                            bool outOk[4],
                            uint16_t &minT,
                            uint16_t &maxT,
                            uint16_t &avgT,
                            uint32_t &outSent,
                            uint32_t &outReceived)
{
    if (daemonStatsMutex && xSemaphoreTake(daemonStatsMutex, portMAX_DELAY) == pdTRUE)
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            outTimes[i] = daemonPktTimes[i];
            outOk[i] = daemonPktOk[i];
        }
        minT = daemonMinTime;
        maxT = daemonMaxTime;
        avgT = (totalReceived > 0) ? static_cast<uint16_t>(totalTimeSum / totalReceived) : 0;
        outSent = totalSent;
        outReceived = totalReceived;
        xSemaphoreGive(daemonStatsMutex);
        return;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        outTimes[i] = 0;
        outOk[i] = false;
    }
    minT = 0;
    maxT = 0;
    avgT = 0;
    outSent = 0;
    outReceived = 0;
}

static void pingDaemonTask(void *param)
{
    (void)param;
    for (;;)
    {
        if (!daemonActive || daemonIpStr[0] == '\0')
        {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        uint8_t octets[4] = {0, 0, 0, 0};
        if (sscanf(daemonIpStr, "%hhu.%hhu.%hhu.%hhu", &octets[0], &octets[1], &octets[2], &octets[3]) != 4)
        {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        IPAddress ip(octets[0], octets[1], octets[2], octets[3]);
        bool ok = Ping.ping(ip, 1);
        uint16_t t = ok ? static_cast<uint16_t>(Ping.averageTime()) : 0;

        if (daemonStatsMutex && xSemaphoreTake(daemonStatsMutex, portMAX_DELAY) == pdTRUE)
        {
            daemonPktOk[daemonPktIndex] = ok;
            daemonPktTimes[daemonPktIndex] = t;
            daemonPktIndex = (daemonPktIndex + 1) % 4;

            totalSent++;
            if (ok)
            {
                totalReceived++;
                totalTimeSum += t;
                if (totalReceived == 1 || t < daemonMinTime)
                {
                    daemonMinTime = t;
                }
                if (t > daemonMaxTime)
                {
                    daemonMaxTime = t;
                }
            }

            xSemaphoreGive(daemonStatsMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void ensureDaemonThread()
{
    if (daemonStatsMutex == nullptr)
    {
        daemonStatsMutex = xSemaphoreCreateMutex();
    }
    if (daemonTaskHandle == nullptr)
    {
        xTaskCreatePinnedToCore(pingDaemonTask, "pingDaemon", 4096, nullptr, 1, &daemonTaskHandle, 1);
    }
}

static bool pickPingTarget(Watchy &watchy, char *outTarget, size_t outTargetSize)
{
    UiTemplates::HistoryPickerSpec picker;
    picker.title = "Select Target:";
    picker.nvsNamespace = kPingHistoryNs;
    picker.exampleLabel = kExampleLabel;
    picker.exampleValue = kExampleTarget;
    picker.newLabel = "New address";
    picker.maxEntries = UiTemplates::HISTORY_MAX_ENTRIES;
    picker.maxEntryLen = UiTemplates::HISTORY_MAX_ENTRY_LEN;

    picker.menuLayout.headerX = 16;
    picker.menuLayout.headerY = 36;
    picker.menuLayout.menuX = 0;
    picker.menuLayout.menuY = 72;
    picker.menuLayout.visibleRowsMax = 4;
    picker.menuLayout.startIndex = 0;
    picker.menuLayout.autoScroll = true;
    picker.menuLayout.font = &FreeMonoBold9pt7b;

    UiTemplates::HistoryPickKind kind = UiTemplates::HistoryPickKind::NewTarget;
    if (!UiTemplates::runHistoryPickerNvs(watchy, picker, outTarget, outTargetSize, kind, sPingHistorySelected))
    {
        return false;
    }
    if (kind == UiTemplates::HistoryPickKind::NewTarget)
    {
        if (outTargetSize > 0)
        {
            outTarget[0] = '\0';
        }
    }
    return true;
}

static bool isIpv4Target(const char *target, uint8_t outOctets[4])
{
    if (target == nullptr || target[0] == '\0')
    {
        return false;
    }
    return NetUtils::parseIp(String(target), outOctets);
}

static void renderStatusLine(Watchy &watchy, const char *line1, const char *line2 = nullptr)
{
    UITextSpec t1{};
    t1.x = 0;
    t1.y = 36;
    t1.font = &FreeMonoBold9pt7b;
    t1.text = (line1 != nullptr) ? String(line1) : String("");

    UITextSpec t2{};
    t2.x = 0;
    t2.y = 56;
    t2.font = &FreeMonoBold9pt7b;
    t2.text = (line2 != nullptr) ? String(line2) : String("");

    UITextSpec texts[2] = {t1, t2};

    UIAppSpec app{};
    app.texts = texts;
    app.textCount = (line2 != nullptr) ? 2 : 1;
    app.controls[0] = sPingControls[0];
    app.controls[1] = sPingControls[1];
    app.controls[2] = sPingControls[2];
    app.controls[3] = sPingControls[3];
    UiSDK::renderApp(watchy, app);
}

static void buildIpString(const uint8_t digits[12], char ipStr[16])
{
    uint8_t si = 0;
    for (uint8_t oct = 0; oct < 4; ++oct)
    {
        for (uint8_t d = 0; d < 3; ++d)
        {
            ipStr[si++] = '0' + digits[oct * 3 + d];
        }
        if (oct < 3)
        {
            ipStr[si++] = '.';
        }
    }
    ipStr[si] = '\0';
}

static void renderPingConfirmTarget(Watchy &watchy, const char *target, bool daemonMode)
{
    const char *t = (target && target[0]) ? target : "(empty)";

    struct DrawData
    {
        const char *target;
        bool daemon;
    } data{t, daemonMode};

    auto draw = [](WatchyGxDisplay &display, void *userData)
    {
        DrawData *d = static_cast<DrawData *>(userData);
        display.setFont(&FreeMonoBold9pt7b);

        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(d->target, 0, 0, &x1, &y1, &w, &h);
        int16_t tx = (WatchyDisplay::WIDTH - static_cast<int16_t>(w)) / 2;
        int16_t ty = 92;
        display.setCursor(tx, ty);
        display.print(d->target);

        const char *label = "daemon mode";
        String boxText = String(d->daemon ? "[x]" : "[ ]") + label;
        int16_t cbx1, cby1;
        uint16_t cbw, cbh;
        display.getTextBounds(boxText, 0, 0, &cbx1, &cby1, &cbw, &cbh);
        int16_t cbX = (WatchyDisplay::WIDTH - static_cast<int16_t>(cbw)) / 2;
        display.setCursor(cbX, ty + static_cast<int16_t>(h) + 34);
        display.print(boxText);
    };

    UICallbackSpec cb{};
    cb.draw = draw;
    cb.userData = &data;

    UIAppSpec app{};
    app.callbacks = &cb;
    app.callbackCount = 1;
    app.controls[0] = sPingControls[0];
    app.controls[1] = sPingControls[1];
    app.controls[2] = sPingControls[2];
    app.controls[3] = sPingControls[3];
    UiSDK::renderApp(watchy, app);
}

static void renderPingEditor(Watchy &watchy, const uint8_t digits[12], uint8_t pos, bool daemonMode)
{
    struct DrawData
    {
        const uint8_t *digits;
        uint8_t pos;
        bool daemon;
    } data{digits, pos, daemonMode};

    auto draw = [](WatchyGxDisplay &display, void *userData)
    {
        DrawData *d = static_cast<DrawData *>(userData);
        display.setFont(&FreeMonoBold9pt7b);

        char ipStr[16];
        buildIpString(d->digits, ipStr);
        const uint8_t ipLen = 15;
        const int16_t baseY = 100;

        int8_t highlightIndex = -1;
        if (d->pos < 12)
        {
            highlightIndex = static_cast<int8_t>(d->pos + d->pos / 3);
        }

        UiTemplates::renderCenteredMonospaceHighlight(display,
                                                      &FreeMonoBold9pt7b,
                                                      ipStr,
                                                      ipLen,
                                                      baseY,
                                                      highlightIndex,
                                                      UiSDK::getWatchfaceFg(BASE_POLARITY),
                                                      UiSDK::getWatchfaceBg(BASE_POLARITY),
                                                      2);

        int16_t x1, y1;
        uint16_t cw, ch;
        display.getTextBounds("0", 0, 0, &x1, &y1, &cw, &ch);
        const int16_t checkboxY = baseY + static_cast<int16_t>(ch) + 16;
        const char *label = "daemon mode";
        String boxText = String(d->daemon ? "[x]" : "[ ]") + label;
        int16_t cbx1, cby1;
        uint16_t cbw, cbh;
        display.getTextBounds(boxText, 0, 0, &cbx1, &cby1, &cbw, &cbh);
        int16_t cbX = (WatchyDisplay::WIDTH - static_cast<int16_t>(cbw)) / 2;
        display.setCursor(cbX, checkboxY);
        display.print(boxText);
    };

    UICallbackSpec cb{};
    cb.draw = draw;
    cb.userData = &data;

    UIAppSpec app{};
    app.callbacks = &cb;
    app.callbackCount = 1;
    app.controls[0] = sPingControls[0];
    app.controls[1] = sPingControls[1];
    app.controls[2] = sPingControls[2];
    app.controls[3] = sPingControls[3];
    UiSDK::renderApp(watchy, app);
}

static void renderPingConfirm(Watchy &watchy, const uint8_t digits[12])
{
    struct DrawData
    {
        const uint8_t *digits;
    } data{digits};

    auto draw = [](WatchyGxDisplay &display, void *userData)
    {
        DrawData *d = static_cast<DrawData *>(userData);
        display.setFont(&FreeMonoBold9pt7b);

        char ipStr[16];
        buildIpString(d->digits, ipStr);

        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(ipStr, 0, 0, &x1, &y1, &w, &h);
        int16_t ipX = (WatchyDisplay::WIDTH - static_cast<int16_t>(w)) / 2;
        int16_t ipY = 100;

        display.setCursor(ipX, ipY);
        display.print(ipStr);

        display.setCursor(58, ipY + h + 10);
        display.println("Confirm ping");
    };

    UICallbackSpec cb{};
    cb.draw = draw;
    cb.userData = &data;

    UIAppSpec app{};
    app.callbacks = &cb;
    app.callbackCount = 1;
    app.controls[0] = sPingControls[0];
    app.controls[1] = sPingControls[1];
    app.controls[2] = sPingControls[2];
    app.controls[3] = sPingControls[3];
    UiSDK::renderApp(watchy, app);
}

static void renderPingResult(Watchy &watchy,
                             const char *targetStr,
                             const char *ipStr,
                             bool badIp,
                             uint8_t sent,
                             uint8_t received,
                             const uint16_t times[4],
                             const bool ok[4],
                             uint16_t minTime,
                             uint16_t maxTime,
                             uint16_t avgTime,
                             bool isDaemonActive,
                             uint32_t daemonTotalSent,
                             uint32_t daemonTotalReceived)
{
    Watchy::display.setFont(&FreeMonoBold9pt7b);

    String body;
    body.reserve(512);

    if (badIp)
    {
        body = "Invalid target\n\nBACK: edit";
    }
    else
    {
        body += "Ping ";
        body += (targetStr && targetStr[0]) ? targetStr : ipStr;
        if (targetStr && targetStr[0] && ipStr && ipStr[0] && strcmp(targetStr, ipStr) != 0)
        {
            body += "\nIP: ";
            body += ipStr;
        }
        body += isDaemonActive ? " (daemon)\n" : "\n";

        if (isDaemonActive)
        {
            body += "Recent pings:\n";
            for (uint8_t i = 0; i < 4; ++i)
            {
                if (ok[i])
                {
                    body += "Reply: time=";
                    body += String(times[i]);
                    body += "ms\n";
                }
                else
                {
                    body += "Reply: timeout\n";
                }
            }

            body += "\nTotal Stats:\n";
            uint32_t lost = daemonTotalSent > daemonTotalReceived ? daemonTotalSent - daemonTotalReceived : 0;
            uint32_t lossPct = daemonTotalSent ? static_cast<uint32_t>((lost * 100) / daemonTotalSent) : 0;
            body += " Sent=" + String(daemonTotalSent);
            body += " Recv=" + String(daemonTotalReceived);
            body += " Lost=" + String(lost);
            body += " (" + String(lossPct) + "%)\n";
        }
        else
        {
            for (uint8_t i = 0; i < sent && i < 4; ++i)
            {
                if (ok[i])
                {
                    body += "Reply ";
                    body += String(i + 1);
                    body += ": time=";
                    body += String(times[i]);
                    body += "ms\n";
                }
                else
                {
                    body += "Reply ";
                    body += String(i + 1);
                    body += ": timeout\n";
                }
            }

            body += "\nStats:\n";
            uint8_t lost = sent > received ? static_cast<uint8_t>(sent - received) : 0;
            uint8_t lossPct = sent ? static_cast<uint8_t>((lost * 100) / sent) : 0;
            body += " Sent=" + String(sent);
            body += " Recv=" + String(received);
            body += " Lost=" + String(lost);
            body += " (" + String(lossPct) + "%)\n";
        }

        body += "RTT ms: ";
        if ((isDaemonActive ? daemonTotalReceived : received) == 0)
        {
            body += "Min=N/A Max=N/A Avg=N/A";
        }
        else
        {
            body += "Min=" + String(minTime);
            body += " Max=" + String(maxTime);
            body += " Avg=" + String(avgTime);
        }
    }

    if (!isDaemonActive)
    {
        body += "\n\nWhen daemon is running,";
        body += "\nyou can press BACK to return.";
        body += "\nDaemon will continue pinging";
        body += "\nin the background.";
    }

    UIScrollableTextSpec scroll{
        20,
        20,
        WatchyDisplay::WIDTH - 40,
        WatchyDisplay::HEIGHT - 40,
        UiSDK::tinyFont6x8(),
        false,
        body,
        nullptr,
        0,
        18,
        10,
        false,
        true,
        true,
    };

    UIAppSpec app{};
    app.scrollTexts = &scroll;
    app.scrollTextCount = 1;
    app.controls[0] = sPingControls[0];
    app.controls[1] = sPingControls[1];
    app.controls[2] = sPingControls[2];
    app.controls[3] = sPingControls[3];
    UiSDK::renderApp(watchy, app);
}

static void pingBackHandler(Watchy *watchy)
{
    if (watchy == nullptr)
    {
        return;
    }

    if (sPing.mode == PingMode::Edit)
    {
        if (sPing.pos == 0)
        {
            sPing.exitRequested = true;
        }
        else
        {
            --sPing.pos;
        }
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::Confirm)
    {
        sPing.mode = PingMode::Edit;
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::ConfirmTarget)
    {
        char nextTarget[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
        if (!pickPingTarget(*watchy, nextTarget, sizeof(nextTarget)))
        {
            sPing.exitRequested = true;
            return;
        }

        for (uint8_t i = 0; i < 12; ++i)
        {
            sPing.digits[i] = 0;
        }
        sPing.pos = 0;

        uint8_t octets[4] = {0, 0, 0, 0};
        if (nextTarget[0] == '\0')
        {
            sPing.lastTargetStr[0] = '\0';
            sPing.mode = PingMode::Edit;
        }
        else if (isIpv4Target(nextTarget, octets))
        {
            for (uint8_t oct = 0; oct < 4; ++oct)
            {
                uint8_t v = octets[oct];
                sPing.digits[oct * 3 + 0] = (v / 100) % 10;
                sPing.digits[oct * 3 + 1] = (v / 10) % 10;
                sPing.digits[oct * 3 + 2] = v % 10;
            }
            strncpy(sPing.lastTargetStr, nextTarget, sizeof(sPing.lastTargetStr) - 1);
            sPing.lastTargetStr[sizeof(sPing.lastTargetStr) - 1] = '\0';
            sPing.mode = PingMode::Edit;
        }
        else
        {
            strncpy(sPing.lastTargetStr, nextTarget, sizeof(sPing.lastTargetStr) - 1);
            sPing.lastTargetStr[sizeof(sPing.lastTargetStr) - 1] = '\0';
            sPing.mode = PingMode::ConfirmTarget;
        }

        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::Result)
    {
        sPing.mode = daemonActive ? PingMode::StopConfirm : PingMode::Edit;
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::StopConfirm)
    {
        sPing.mode = PingMode::Result;
        sPing.needsRender = true;
        return;
    }
}

static void pingUpHandler(Watchy *watchy)
{
    
    if (sPing.mode == PingMode::Edit)
    {
        if (sPing.pos < 12)
        {
            const bool isHundreds = (sPing.pos % 3) == 0;
            if (isHundreds)
            {
                sPing.digits[sPing.pos] = (sPing.digits[sPing.pos] >= 2) ? 0 : (sPing.digits[sPing.pos] + 1);
            }
            else
            {
                sPing.digits[sPing.pos] = (sPing.digits[sPing.pos] + 1) % 10;
            }
        }
        else
        {
            sPing.daemonMode = !sPing.daemonMode;
        }
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::ConfirmTarget)
    {
        sPing.daemonMode = !sPing.daemonMode;
        sPing.needsRender = true;
        return;
    }
}

static void pingDownHandler(Watchy *watchy)
{
    
    if (sPing.mode == PingMode::Edit)
    {
        if (sPing.pos < 12)
        {
            const bool isHundreds = (sPing.pos % 3) == 0;
            if (isHundreds)
            {
                sPing.digits[sPing.pos] = (sPing.digits[sPing.pos] == 0) ? 2 : (sPing.digits[sPing.pos] - 1);
            }
            else
            {
                sPing.digits[sPing.pos] = (sPing.digits[sPing.pos] == 0) ? 9 : (sPing.digits[sPing.pos] - 1);
            }
        }
        else
        {
            sPing.daemonMode = !sPing.daemonMode;
        }
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::ConfirmTarget)
    {
        sPing.daemonMode = !sPing.daemonMode;
        sPing.needsRender = true;
        return;
    }
}

static void pingMenuHandler(Watchy *watchy)
{
    UiTemplates::waitForAllButtonsReleased(50, 100);
    if (watchy == nullptr)
    {
        return;
    }

    if (sPing.mode == PingMode::Edit)
    {
        if (sPing.pos < 12)
        {
            ++sPing.pos;
            sPing.needsRender = true;
            return;
        }

        const int8_t choice = UiTemplates::runToast2Option(*watchy, "Confirm ping", "Yes", "No", 1);
        if (choice != 0)
        {
            return;
        }

        uint8_t octets[4] = {0, 0, 0, 0};
        sPing.resultBadIp = false;
        for (uint8_t oct = 0; oct < 4; ++oct)
        {
            uint16_t val = sPing.digits[oct * 3] * 100 + sPing.digits[oct * 3 + 1] * 10 + sPing.digits[oct * 3 + 2];
            if (val > 255)
            {
                sPing.resultBadIp = true;
                break;
            }
            octets[oct] = static_cast<uint8_t>(val);
        }

        snprintf(sPing.lastIpStr, sizeof(sPing.lastIpStr), "%u.%u.%u.%u", octets[0], octets[1], octets[2], octets[3]);
        strncpy(sPing.lastTargetStr, sPing.lastIpStr, sizeof(sPing.lastTargetStr) - 1);
        sPing.lastTargetStr[sizeof(sPing.lastTargetStr) - 1] = '\0';

        if (!sPing.resultBadIp && sPing.lastTargetStr[0] != '\0')
        {
            UiTemplates::historyAddUniqueNvs(kPingHistoryNs,
                                             sPing.lastTargetStr,
                                             UiTemplates::HISTORY_MAX_ENTRIES,
                                             UiTemplates::HISTORY_MAX_ENTRY_LEN);
        }

        if (!sPing.resultBadIp)
        {
            bool wifiOk = NetUtils::ensureWiFi(*watchy, 1, 8000);
            renderStatusLine(*watchy, wifiOk ? "Pinging..." : "No WiFi");

            if (wifiOk)
            {
                IPAddress ip(octets[0], octets[1], octets[2], octets[3]);

                sPing.recvCount = 0;
                sPing.minTime = 0;
                sPing.maxTime = 0;
                sPing.avgTime = 0;
                totalSent = 0;
                totalReceived = 0;
                totalTimeSum = 0;
                daemonMinTime = 0;
                daemonMaxTime = 0;

                for (uint8_t i = 0; i < PingState::PING_COUNT; ++i)
                {
                    sPing.pktOk[i] = false;
                    sPing.pktTimes[i] = 0;
                }

                if (sPing.daemonMode)
                {
                    if (daemonStatsMutex && xSemaphoreTake(daemonStatsMutex, portMAX_DELAY) == pdTRUE)
                    {
                        resetDaemonStatsLocked();
                        xSemaphoreGive(daemonStatsMutex);
                    }
                    else
                    {
                        resetDaemonStatsLocked();
                    }

                    daemonActive = true;
                    strncpy(daemonIpStr, sPing.lastIpStr, sizeof(daemonIpStr) - 1);
                    daemonIpStr[sizeof(daemonIpStr) - 1] = '\0';
                    sPing.sentCount = PingState::PING_COUNT;
                    ensureDaemonThread();
                }
                else
                {
                    for (uint8_t i = 0; i < PingState::PING_COUNT; ++i)
                    {
                        bool ok = Ping.ping(ip, 1);
                        sPing.pktOk[i] = ok;
                        if (ok)
                        {
                            uint16_t t = static_cast<uint16_t>(Ping.averageTime());
                            sPing.pktTimes[i] = t;
                            if (sPing.recvCount == 0 || t < sPing.minTime)
                            {
                                sPing.minTime = t;
                            }
                            if (t > sPing.maxTime)
                            {
                                sPing.maxTime = t;
                            }
                            sPing.avgTime = static_cast<uint16_t>(sPing.avgTime + t);
                            sPing.recvCount++;
                        }
                        delay(1000);
                    }
                    if (sPing.recvCount > 0)
                    {
                        sPing.avgTime = static_cast<uint16_t>(sPing.avgTime / sPing.recvCount);
                    }
                    WiFi.mode(WIFI_OFF);
                    btStop();
                    daemonActive = false;
                }
            }
            else
            {
                sPing.recvCount = 0;
            }
        }

        sPing.mode = PingMode::Result;
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::ConfirmTarget)
    {
        const int8_t choice = UiTemplates::runToast2Option(*watchy, "Confirm ping", "Yes", "No", 1);
        if (choice != 0)
        {
            return;
        }

        sPing.resultBadIp = false;
        uint8_t octets[4] = {0, 0, 0, 0};
        bool haveIp = false;

        if (isIpv4Target(sPing.lastTargetStr, octets))
        {
            snprintf(sPing.lastIpStr, sizeof(sPing.lastIpStr), "%u.%u.%u.%u", octets[0], octets[1], octets[2], octets[3]);
            haveIp = true;
        }
        else
        {
            bool wifiOk = NetUtils::ensureWiFi(*watchy, 1, 8000);
            renderStatusLine(*watchy, wifiOk ? "Resolving..." : "No WiFi");
            if (!wifiOk)
            {
                sPing.resultBadIp = true;
                sPing.mode = PingMode::Result;
                sPing.needsRender = true;
                return;
            }

            IPAddress resolved;
            if (!WiFi.hostByName(sPing.lastTargetStr, resolved))
            {
                sPing.resultBadIp = true;
                sPing.mode = PingMode::Result;
                sPing.needsRender = true;
                return;
            }

            String ipS = resolved.toString();
            strncpy(sPing.lastIpStr, ipS.c_str(), sizeof(sPing.lastIpStr) - 1);
            sPing.lastIpStr[sizeof(sPing.lastIpStr) - 1] = '\0';
            octets[0] = resolved[0];
            octets[1] = resolved[1];
            octets[2] = resolved[2];
            octets[3] = resolved[3];
            haveIp = true;
        }

        if (!haveIp)
        {
            sPing.resultBadIp = true;
            sPing.mode = PingMode::Result;
            sPing.needsRender = true;
            return;
        }

        if (sPing.lastTargetStr[0] != '\0')
        {
            UiTemplates::historyAddUniqueNvs(kPingHistoryNs,
                                             sPing.lastTargetStr,
                                             UiTemplates::HISTORY_MAX_ENTRIES,
                                             UiTemplates::HISTORY_MAX_ENTRY_LEN);
        }

        bool wifiOk = NetUtils::ensureWiFi(*watchy, 1, 8000);
        renderStatusLine(*watchy, wifiOk ? "Pinging..." : "No WiFi");
        if (wifiOk)
        {
            IPAddress ip(octets[0], octets[1], octets[2], octets[3]);

            sPing.recvCount = 0;
            sPing.minTime = 0;
            sPing.maxTime = 0;
            sPing.avgTime = 0;
            totalSent = 0;
            totalReceived = 0;
            totalTimeSum = 0;
            daemonMinTime = 0;
            daemonMaxTime = 0;
            for (uint8_t i = 0; i < PingState::PING_COUNT; ++i)
            {
                sPing.pktOk[i] = false;
                sPing.pktTimes[i] = 0;
            }

            if (sPing.daemonMode)
            {
                if (daemonStatsMutex && xSemaphoreTake(daemonStatsMutex, portMAX_DELAY) == pdTRUE)
                {
                    resetDaemonStatsLocked();
                    xSemaphoreGive(daemonStatsMutex);
                }
                else
                {
                    resetDaemonStatsLocked();
                }
                daemonActive = true;
                strncpy(daemonIpStr, sPing.lastIpStr, sizeof(daemonIpStr) - 1);
                daemonIpStr[sizeof(daemonIpStr) - 1] = '\0';
                sPing.sentCount = PingState::PING_COUNT;
                ensureDaemonThread();
            }
            else
            {
                for (uint8_t i = 0; i < PingState::PING_COUNT; ++i)
                {
                    bool ok = Ping.ping(ip, 1);
                    sPing.pktOk[i] = ok;
                    if (ok)
                    {
                        uint16_t t = static_cast<uint16_t>(Ping.averageTime());
                        sPing.pktTimes[i] = t;
                        if (sPing.recvCount == 0 || t < sPing.minTime)
                        {
                            sPing.minTime = t;
                        }
                        if (t > sPing.maxTime)
                        {
                            sPing.maxTime = t;
                        }
                        sPing.avgTime = static_cast<uint16_t>(sPing.avgTime + t);
                        sPing.recvCount++;
                    }
                    delay(1000);
                }
                if (sPing.recvCount > 0)
                {
                    sPing.avgTime = static_cast<uint16_t>(sPing.avgTime / sPing.recvCount);
                }
                WiFi.mode(WIFI_OFF);
                btStop();
                daemonActive = false;
            }
        }

        sPing.mode = PingMode::Result;
        sPing.needsRender = true;
        return;
    }

    if (sPing.mode == PingMode::StopConfirm)
    {
        daemonActive = false;
        daemonIpStr[0] = '\0';
        totalSent = 0;
        totalReceived = 0;
        totalTimeSum = 0;
        daemonMinTime = 0;
        daemonMaxTime = 0;
        WiFi.mode(WIFI_OFF);
        btStop();
        sPing.mode = PingMode::Result;
        sPing.needsRender = true;
        return;
    }
}

void Watchy::showPing()
{
    guiState = APP_STATE;
    sPing = PingState{};

    char selectedTarget[UiTemplates::HISTORY_MAX_ENTRY_LEN + 1] = {0};
    if (!pickPingTarget(*this, selectedTarget, sizeof(selectedTarget)))
    {
        showMenu(menuIndex);
        return;
    }

    if (selectedTarget[0] != '\0')
    {
        uint8_t octets[4] = {0, 0, 0, 0};
        if (isIpv4Target(selectedTarget, octets))
        {
            for (uint8_t oct = 0; oct < 4; ++oct)
            {
                uint8_t v = octets[oct];
                sPing.digits[oct * 3 + 0] = (v / 100) % 10;
                sPing.digits[oct * 3 + 1] = (v / 10) % 10;
                sPing.digits[oct * 3 + 2] = v % 10;
            }
            strncpy(sPing.lastTargetStr, selectedTarget, sizeof(sPing.lastTargetStr) - 1);
            sPing.lastTargetStr[sizeof(sPing.lastTargetStr) - 1] = '\0';
            sPing.mode = PingMode::Edit;
        }
        else
        {
            strncpy(sPing.lastTargetStr, selectedTarget, sizeof(sPing.lastTargetStr) - 1);
            sPing.lastTargetStr[sizeof(sPing.lastTargetStr) - 1] = '\0';
            sPing.mode = PingMode::ConfirmTarget;
        }
    }
    else
    {
        sPing.mode = PingMode::Edit;
    }
    sPing.needsRender = true;

    UiTemplates::waitForAllButtonsReleased();

    setButtonHandlers(pingBackHandler, pingUpHandler, pingMenuHandler, pingDownHandler);

    uint32_t nextDaemonRefreshMs = 0;
    while (!sPing.exitRequested)
    {
        UiSDK::renderControlsRow(*this, sPingControls);

        // Global BACK long-press (2s) to jump to menu (when daemon not running).
        if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW)
        {
            if (!sPing.backHeld)
            {
                sPing.backHeld = true;
                sPing.backHoldStartMs = millis();
            }
            else if (millis() - sPing.backHoldStartMs >= 2000)
            {
                if (!daemonActive)
                {
                    WiFi.mode(WIFI_OFF);
                    btStop();
                    break;
                }
            }
        }
        else
        {
            sPing.backHeld = false;
        }

        if (daemonActive)
        {
            uint16_t timesCopy[4];
            bool okCopy[4];
            uint16_t minCopy, maxCopy, avgCopy;
            uint32_t sentCopy, recvCopy;
            copyDaemonStats(timesCopy, okCopy, minCopy, maxCopy, avgCopy, sentCopy, recvCopy);
            for (uint8_t i = 0; i < PingState::PING_COUNT; ++i)
            {
                sPing.pktTimes[i] = timesCopy[i];
                sPing.pktOk[i] = okCopy[i];
            }
            sPing.minTime = minCopy;
            sPing.maxTime = maxCopy;
            sPing.avgTime = avgCopy;
            sPing.recvCount = (recvCopy > 255) ? 255 : static_cast<uint8_t>(recvCopy);
            totalSent = sentCopy;
            totalReceived = recvCopy;

            const uint32_t now = millis();
            if (sPing.mode == PingMode::Result && (nextDaemonRefreshMs == 0 || now >= nextDaemonRefreshMs))
            {
                nextDaemonRefreshMs = now + 500;
                sPing.needsRender = true;
            }
        }

        if (sPing.needsRender)
        {
            sPing.needsRender = false;
            if (sPing.mode == PingMode::Edit)
            {
                renderPingEditor(*this, sPing.digits, sPing.pos, sPing.daemonMode);
            }
            else if (sPing.mode == PingMode::Confirm)
            {
                renderPingConfirm(*this, sPing.digits);
            }
            else if (sPing.mode == PingMode::ConfirmTarget)
            {
                renderPingConfirmTarget(*this, sPing.lastTargetStr, sPing.daemonMode);
            }
            else if (sPing.mode == PingMode::Result)
            {
                renderPingResult(*this,
                                 sPing.lastTargetStr,
                                 sPing.lastIpStr,
                                 sPing.resultBadIp,
                                 sPing.sentCount,
                                 sPing.recvCount,
                                 sPing.pktTimes,
                                 sPing.pktOk,
                                 sPing.minTime,
                                 sPing.maxTime,
                                 sPing.avgTime,
                                 daemonActive,
                                 totalSent,
                                 totalReceived);
            }
            else
            {
                UITextSpec texts[6];
                texts[0] = {10, 36, 0, 0, &FreeMonoBold9pt7b, false, false, "Stop Daemon?"};
                texts[1] = {10, 65, 0, 0, &FreeMonoBold9pt7b, false, false, "Continuous ping"};
                texts[2] = {10, 85, 0, 0, &FreeMonoBold9pt7b, false, false, "will be stopped"};
                texts[3] = {10, 120, 0, 0, &FreeMonoBold9pt7b, false, false, "ACCEPT: Stop"};
                texts[4] = {10, 145, 0, 0, &FreeMonoBold9pt7b, false, false, "BACK: Continue"};
                UIAppSpec app{};
                app.texts = texts;
                app.textCount = 5;
                app.controls[0] = sPingControls[0];
                app.controls[1] = sPingControls[1];
                app.controls[2] = sPingControls[2];
                app.controls[3] = sPingControls[3];
                UiSDK::renderApp(*this, app);
            }
        }

        delay(10);
    }

    clearButtonHandlers();
    delay(100);
    showMenu(menuIndex);
}
