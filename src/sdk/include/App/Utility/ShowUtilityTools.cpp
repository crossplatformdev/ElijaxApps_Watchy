#include "WatchyUi.h"
#include "Watchy.h"

#include "UtilityToolApps.h"

namespace {

void showUtilityToolImpl(uint8_t rawTool, Watchy *watchy) {
  using namespace WatchyUtilityTools;
  Tool tool = rawTool < ToolCount ? static_cast<Tool>(rawTool) : CoinFlip;
  switch (tool) {
  case CoinFlip: runCoinFlip(); break;
  case D6Dice: runD6Dice(); break;
  case D20Dice: runD20Dice(); break;
  case RandomNumber: runRandomNumber(); break;
  case DecisionMaker: runDecisionMaker(); break;
  case PasswordGenerator: runPasswordGenerator(); break;
  case UuidGenerator: runUuidGenerator(); break;
  case I2cScanner: runI2cScanner(); break;
  case ChipInfo: runChipInfo(); break;
  case HeapMonitor: runHeapMonitor(); break;
  case WakeReason: runWakeReason(); break;
  case ResetCause: runResetCause(); break;
  case ButtonTester: runButtonTester(); break;
  case VibrationLab: runVibrationLab(); break;
  case ScreenRuler: runScreenRuler(); break;
  case TemperatureConverter: runTemperatureConverter(); break;
  case LengthConverter: runLengthConverter(); break;
  case WeightConverter: runWeightConverter(); break;
  case BaseConverter: runBaseConverter(); break;
  case PaceConverter: runPaceConverter(); break;
  default: break;
  }
  if (watchy != nullptr) {
    watchy->showMenu(menuIndex, false);
  } else {
    WatchySdk::showMenu(menuIndex, false);
  }
}

} // namespace

void Watchy::showUtilityTool(uint8_t rawTool) {
  showUtilityToolImpl(rawTool, this);
}

void WatchySdk::showUtilityTool(uint8_t rawTool) {
  showUtilityToolImpl(rawTool, nullptr);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {

void renderUtilityPreview(uint8_t rawTool, uint8_t view) {
  using namespace WatchyUtilityTools;
  switch (rawTool < ToolCount ? static_cast<Tool>(rawTool) : CoinFlip) {
  case CoinFlip: renderCoinFlipPreview(view); break;
  case D6Dice: renderD6DicePreview(view); break;
  case D20Dice: renderD20DicePreview(view); break;
  case RandomNumber: renderRandomNumberPreview(view); break;
  case DecisionMaker: renderDecisionMakerPreview(view); break;
  case PasswordGenerator: renderPasswordGeneratorPreview(view); break;
  case UuidGenerator: renderUuidGeneratorPreview(view); break;
  case I2cScanner: renderI2cScannerPreview(view); break;
  case ChipInfo: renderChipInfoPreview(view); break;
  case HeapMonitor: renderHeapMonitorPreview(view); break;
  case WakeReason: renderWakeReasonPreview(view); break;
  case ResetCause: renderResetCausePreview(view); break;
  case ButtonTester: renderButtonTesterPreview(view); break;
  case VibrationLab: renderVibrationLabPreview(view); break;
  case ScreenRuler: renderScreenRulerPreview(view); break;
  case TemperatureConverter: renderTemperatureConverterPreview(view); break;
  case LengthConverter: renderLengthConverterPreview(view); break;
  case WeightConverter: renderWeightConverterPreview(view); break;
  case BaseConverter: renderBaseConverterPreview(view); break;
  case PaceConverter: renderPaceConverterPreview(view); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif
