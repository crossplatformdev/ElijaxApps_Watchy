#ifndef WATCHY_TIME_SUPPORT_H
#define WATCHY_TIME_SUPPORT_H

#include <stddef.h>
#include <stdint.h>

#include "WatchyUi.h"

namespace WatchyTimeTools {

void configureButtons();
void pulseMotor(uint16_t durationMs);
void formatDuration(char *output, size_t outputSize, uint32_t totalSeconds);
void drawTimer(const char *title, uint32_t seconds, const char *state,
               const char *controls, const char *detail = nullptr,
               bool valueOnly = false, float progress = -1.0f);
void runCountdownTimer(uint32_t initialSeconds, const char *title,
                       bool pomodoro);

} // namespace WatchyTimeTools

#endif