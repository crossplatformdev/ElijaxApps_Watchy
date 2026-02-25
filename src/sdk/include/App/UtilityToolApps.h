#ifndef WATCHY_UTILITY_TOOL_APPS_H
#define WATCHY_UTILITY_TOOL_APPS_H

#include <WatchySdk.h>

namespace WatchyUtilityTools {

enum Tool : uint8_t {
  CoinFlip,
  D6Dice,
  D20Dice,
  RandomNumber,
  DecisionMaker,
  PasswordGenerator,
  UuidGenerator,
  I2cScanner,
  ChipInfo,
  HeapMonitor,
  WakeReason,
  ResetCause,
  ButtonTester,
  VibrationLab,
  ScreenRuler,
  TemperatureConverter,
  LengthConverter,
  WeightConverter,
  BaseConverter,
  PaceConverter,
  ToolCount
};

void runCoinFlip();
void runD6Dice();
void runD20Dice();
void runRandomNumber();
void runDecisionMaker();
void runPasswordGenerator();
void runUuidGenerator();
void runI2cScanner();
void runChipInfo();
void runHeapMonitor();
void runWakeReason();
void runResetCause();
void runButtonTester();
void runVibrationLab();
void runScreenRuler();
void runTemperatureConverter();
void runLengthConverter();
void runWeightConverter();
void runBaseConverter();
void runPaceConverter();

#ifdef WATCHY_DETERMINISTIC_GALLERY
void renderCoinFlipPreview(uint8_t view);
void renderD6DicePreview(uint8_t view);
void renderD20DicePreview(uint8_t view);
void renderRandomNumberPreview(uint8_t view);
void renderDecisionMakerPreview(uint8_t view);
void renderPasswordGeneratorPreview(uint8_t view);
void renderUuidGeneratorPreview(uint8_t view);
void renderI2cScannerPreview(uint8_t view);
void renderChipInfoPreview(uint8_t view);
void renderHeapMonitorPreview(uint8_t view);
void renderWakeReasonPreview(uint8_t view);
void renderResetCausePreview(uint8_t view);
void renderButtonTesterPreview(uint8_t view);
void renderVibrationLabPreview(uint8_t view);
void renderScreenRulerPreview(uint8_t view);
void renderTemperatureConverterPreview(uint8_t view);
void renderLengthConverterPreview(uint8_t view);
void renderWeightConverterPreview(uint8_t view);
void renderBaseConverterPreview(uint8_t view);
void renderPaceConverterPreview(uint8_t view);
#endif

} // namespace WatchyUtilityTools

#endif