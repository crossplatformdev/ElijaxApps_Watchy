#ifndef HEART_RATE_H
#define HEART_RATE_H

#include <Watchy.h>

extern RTC_DATA_ATTR uint8_t heartRateBpm;
extern RTC_DATA_ATTR bool heartRateValid;

constexpr uint8_t HEART_RATE_BACKGROUND_WAKE_SECONDS = 5;

typedef void (*HeartRateUpdateCallback)(bool heartFilled, bool measuring);

bool measureHeartRateSilently(
    uint32_t durationMs = 15000,
    HeartRateUpdateCallback updateCallback = nullptr);
void stopHeartRateMeasurement();
bool waitForHeartRateMeasurement(uint32_t timeoutMs = UINT32_MAX);
bool isHeartRateMeasurementRunning();
void setWatchfaceHeartRateMonitoring(bool enabled);
bool serviceWatchfaceHeartRateMonitoring();
bool isWatchfaceHeartRateMonitoringActive();
void drawHeartIcon(int16_t centerX, int16_t centerY, bool filled,
                   uint16_t color);

#endif