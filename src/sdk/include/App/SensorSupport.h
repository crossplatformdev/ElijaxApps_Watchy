#ifndef WATCHY_SENSOR_SUPPORT_H
#define WATCHY_SENSOR_SUPPORT_H

#include <WatchySdk.h>

#include "WatchyUi.h"

namespace WatchySensorTools {

constexpr uint32_t dailyStepGoal = 10000;
constexpr uint32_t batteryViewRefreshIntervalMs = 60000;

using StaticRenderer = void (*)();
using LiveRenderer = void (*)(const Accel &acceleration);

void useBodyText(int16_t x = 4, int16_t y = 42);
bool readAcceleration(Accel &acceleration);
void drawAxisValue(const char *label, float value, float maximum, int16_t y,
                   bool integer = false);
void drawSensorReadFailure();
bool acquireLiveSensor(const char *title);
void releaseLiveSensor();
void runStaticTool(const char *title, StaticRenderer renderer,
                   uint32_t refreshIntervalMs =
                       WatchyUi::Screen::liveViewRefreshIntervalMs);
void runLiveTool(const char *title, bool requireSample,
                 LiveRenderer renderer);

} // namespace WatchySensorTools

#endif