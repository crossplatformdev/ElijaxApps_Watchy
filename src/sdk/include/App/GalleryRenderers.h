#ifndef WATCHY_GALLERY_RENDERERS_H
#define WATCHY_GALLERY_RENDERERS_H

#include <Arduino.h>
#include <TimeLib.h>

namespace WatchyDemo {

#ifdef WATCHY_DETERMINISTIC_GALLERY

void renderClockPreview(uint8_t tool, uint8_t view, const tmElements_t &fixedTime);
void renderTimeToolPreview(uint8_t tool, uint8_t view);
void renderSensorPreview(uint8_t tool, uint8_t view);
void renderUtilityPreview(uint8_t tool, uint8_t view);
void renderMorseLetterPreview(uint8_t view);
void renderMorseCodePreview(uint8_t view);
void renderPongPreview(uint8_t view);
void renderSnakePreview(uint8_t view);
void renderOthelloPreview(uint8_t view);
void renderMiniGamePreview(uint8_t game, uint8_t view);
void renderHeartRatePreview(uint8_t view);
void renderHealthcarePreview(uint8_t tool, uint8_t view);
void renderSafetyPreview(uint8_t tool, uint8_t view);
void renderHealthReminderPreview(uint8_t tool, uint8_t view);
void renderHealthSupportPreview(uint8_t tool, uint8_t view);
void renderAboutPreview(uint8_t view);
void renderSetTimePreview(uint8_t view);
void renderWifiSetupPreview(uint8_t view);
void renderWatchfaceSelectorPreview(uint8_t view);
void renderThemePreview(uint8_t view);
void renderBuzzPreview(uint8_t view);
void renderAccelerometerPreview(uint8_t view);
void renderSyncNtpPreview(uint8_t view);
void renderSunRisePreview(uint8_t view, const tmElements_t &fixedTime);
void renderMoonRisePreview(uint8_t view, const tmElements_t &fixedTime);
void renderMoonPhasePreview(uint8_t view, const tmElements_t &fixedTime);
void renderTidesPreview(uint8_t view, const tmElements_t &fixedTime);
void renderNetworkPreview(uint8_t tool, uint8_t view);
void renderBluetoothPreview(uint8_t tool, uint8_t view);
void renderMenuPreview(int8_t categoryIndex);

#endif

} // namespace WatchyDemo

#endif