#ifndef HEART_RATE_H
#define HEART_RATE_H

#include <WatchySdk.h>

extern RTC_DATA_ATTR uint8_t heartRateBpm;
extern RTC_DATA_ATTR bool heartRateValid;
extern RTC_DATA_ATTR bool heartRateBeatDetected;

constexpr uint8_t HEART_RATE_MINUTE_REFRESH_GRACE_SECONDS = 2;

typedef void (*HeartRateUpdateCallback)(bool heartFilled, bool measuring);

bool measureHeartRateSilently(
    uint32_t durationMs = 0,
    HeartRateUpdateCallback updateCallback = nullptr);
void stopHeartRateMeasurement();
bool waitForHeartRateMeasurement(uint32_t timeoutMs = 3000);
void abortHeartRateMeasurement();
bool isHeartRateMeasurementRunning();
void setWatchfaceHeartRateMonitoring(bool enabled);
bool serviceWatchfaceHeartRateMonitoring();
bool isWatchfaceHeartRateMonitoringActive();
uint64_t watchfaceHeartRateWakeMask();
void drawHeartIcon(int16_t centerX, int16_t centerY, bool filled,
                   uint16_t color);

#endif