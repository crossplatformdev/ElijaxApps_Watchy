#ifndef WATCHY_GALLERY_RENDERERS_H
#define WATCHY_GALLERY_RENDERERS_H

#include <Arduino.h>
#include <TimeLib.h>

namespace WatchyDemo {

#ifdef WATCHY_DETERMINISTIC_GALLERY

void renderClockPreview(uint8_t tool, const tmElements_t &fixedTime);
void renderTimeToolPreview(uint8_t tool);
void renderSensorPreview(uint8_t tool);
void renderUtilityPreview(uint8_t tool);
void renderMorseLetterPreview();
void renderMorseCodePreview();
void renderPongPreview();
void renderSnakePreview();
void renderOthelloPreview();
void renderMiniGamePreview(uint8_t game);
void renderHeartRatePreview();
void renderHealthcarePreview(uint8_t tool);
void renderSafetyPreview(uint8_t tool);
void renderHealthReminderPreview(uint8_t tool);
void renderHealthSupportPreview(uint8_t tool);
void renderAboutPreview();
void renderSetTimePreview();
void renderWifiSetupPreview();
void renderWatchfaceSelectorPreview();
void renderThemePreview();
void renderBuzzPreview();
void renderAccelerometerPreview();
void renderSyncNtpPreview();
void renderSunRisePreview(const tmElements_t &fixedTime);
void renderMoonRisePreview(const tmElements_t &fixedTime);
void renderMoonPhasePreview(const tmElements_t &fixedTime);
void renderTidesPreview(const tmElements_t &fixedTime);
void renderNetworkPreview(uint8_t tool);
void renderBluetoothPreview(uint8_t tool);
void renderMenuPreview(int8_t categoryIndex);

#endif

} // namespace WatchyDemo

#endif