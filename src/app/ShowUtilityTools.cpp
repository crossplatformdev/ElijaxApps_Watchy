#include <Watchy.h>
#include <esp_system.h>
#include <math.h>
#include "AppDisplay.h"

namespace {

enum UtilityTool : uint8_t {
  COIN_FLIP,
  D6_DICE,
  D20_DICE,
  RANDOM_NUMBER,
  DECISION_MAKER,
  PASSWORD_GENERATOR,
  UUID_GENERATOR,
  I2C_SCANNER,
  CHIP_INFO,
  HEAP_MONITOR,
  WAKE_REASON,
  RESET_CAUSE,
  BUTTON_TESTER,
  VIBRATION_LAB,
  SCREEN_RULER,
  TEMPERATURE_CONVERTER,
  LENGTH_CONVERTER,
  WEIGHT_CONVERTER,
  BASE_CONVERTER,
  PACE_CONVERTER,
  UTILITY_TOOL_COUNT
};

void useSmallText(int16_t x = 4, int16_t y = 38) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(x, y);
}

void randomPassword(char *output, size_t length) {
  constexpr char characters[] =
      "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%";
  for (size_t index = 0; index < length; index++) {
    output[index] = characters[esp_random() % (sizeof(characters) - 1)];
  }
  output[length] = '\0';
}

void randomUuid(char *output) {
  uint8_t bytes[16];
  for (uint8_t index = 0; index < 16; index += 4) {
    uint32_t value = esp_random();
    memcpy(bytes + index, &value, 4);
  }
  bytes[6] = (bytes[6] & 0x0f) | 0x40;
  bytes[8] = (bytes[8] & 0x3f) | 0x80;
  snprintf(output, 37,
           "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
           "%02x%02x%02x%02x%02x%02x",
           bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5],
           bytes[6], bytes[7], bytes[8], bytes[9], bytes[10], bytes[11],
           bytes[12], bytes[13], bytes[14], bytes[15]);
}

void drawGenerator(uint8_t tool, const char *text) {
  const char *title = "RANDOM";
  if (tool == COIN_FLIP) title = "COIN FLIP";
  else if (tool == D6_DICE) title = "D6 DICE";
  else if (tool == D20_DICE) title = "D20 DICE";
  else if (tool == DECISION_MAKER) title = "DECISION MAKER";
  else if (tool == PASSWORD_GENERATOR) title = "PASSWORD GEN";
  else if (tool == UUID_GENERATOR) title = "UUID GENERATOR";
  beginAppDisplay(title);
  useSmallText(6, 70);
  if (tool <= DECISION_MAKER) {
    Watchy::display.setTextSize(4);
  } else if (tool == PASSWORD_GENERATOR) {
    Watchy::display.setTextSize(2);
  }
  if (tool == UUID_GENERATOR) {
    char firstLine[19];
    memcpy(firstLine, text, 18);
    firstLine[18] = '\0';
    Watchy::display.println(firstLine);
    Watchy::display.setCursor(6, 90);
    Watchy::display.println(text + 18);
  } else {
    Watchy::display.println(text);
  }
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(6, 170);
  Watchy::display.println("SELECT: NEW  BACK: EXIT");
  finishAppDisplay();
}

void runGenerator(uint8_t tool) {
  WatchyUi::Input::begin();
  char output[40];
  while (true) {
    switch (tool) {
    case COIN_FLIP: snprintf(output, sizeof(output), "%s", esp_random() & 1 ? "HEADS" : "TAILS"); break;
    case D6_DICE: snprintf(output, sizeof(output), "%lu", static_cast<unsigned long>(esp_random() % 6 + 1)); break;
    case D20_DICE: snprintf(output, sizeof(output), "%lu", static_cast<unsigned long>(esp_random() % 20 + 1)); break;
    case RANDOM_NUMBER: snprintf(output, sizeof(output), "%lu", static_cast<unsigned long>(esp_random() % 1000)); break;
    case DECISION_MAKER: snprintf(output, sizeof(output), "%s", esp_random() & 1 ? "YES" : "NO"); break;
    case PASSWORD_GENERATOR: randomPassword(output, 12); break;
    case UUID_GENERATOR: randomUuid(output); break;
    default: output[0] = '\0'; break;
    }
    drawGenerator(tool, output);
    while (true) {
      WatchyUi::Event event = WatchyUi::Input::wait();
      if (event == WatchyUi::Event::BACK) {
        return;
      }
      if (event == WatchyUi::Event::SELECT) {
        break;
      }
    }
  }
}

void drawI2cScanner() {
  beginAppDisplay("I2C SCANNER");
  useSmallText(2, 34);
  uint8_t found = 0;
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Watchy::display.print("0x");
      if (address < 16) Watchy::display.print('0');
      Watchy::display.print(address, HEX);
      Watchy::display.print(' ');
      found++;
      if (found % 6 == 0) Watchy::display.println();
    }
  }
  Watchy::display.setCursor(2, 145);
  Watchy::display.print("DEVICES FOUND: ");
  Watchy::display.println(found);
  Watchy::display.setCursor(2, 175);
  Watchy::display.println("Read-only address probe");
  finishAppDisplay();
}

void drawChipInfo() {
  esp_chip_info_t info;
  esp_chip_info(&info);
  beginAppDisplay("CHIP INFO");
  useSmallText(6, 42);
  Watchy::display.print("MODEL       ESP32-S3");
  Watchy::display.setCursor(6, 65);
  Watchy::display.print("CORES       "); Watchy::display.println(info.cores);
  Watchy::display.setCursor(6, 88);
  Watchy::display.print("REVISION    "); Watchy::display.println(info.revision);
  Watchy::display.setCursor(6, 111);
  Watchy::display.print("CPU MHz     "); Watchy::display.println(getCpuFrequencyMhz());
  Watchy::display.setCursor(6, 134);
  Watchy::display.print("FLASH       "); Watchy::display.print(ESP.getFlashChipSize() / 1048576); Watchy::display.println(" MB");
  Watchy::display.setCursor(6, 157);
  Watchy::display.print("SDK         "); Watchy::display.println(ESP.getSdkVersion());
  finishAppDisplay();
}

void drawHeap() {
  beginAppDisplay("HEAP MONITOR");
  useSmallText(4, 46);
  Watchy::display.print("FREE HEAP   "); Watchy::display.println(ESP.getFreeHeap());
  Watchy::display.setCursor(4, 76);
  Watchy::display.print("MIN FREE    "); Watchy::display.println(ESP.getMinFreeHeap());
  Watchy::display.setCursor(4, 106);
  Watchy::display.print("MAX BLOCK   "); Watchy::display.println(ESP.getMaxAllocHeap());
  Watchy::display.setCursor(4, 136);
  Watchy::display.print("HEAP SIZE   "); Watchy::display.println(ESP.getHeapSize());
  Watchy::display.setCursor(4, 170);
  Watchy::display.println("Bytes; no PSRAM on Watchy v3");
  finishAppDisplay();
}

const char *wakeReasonName(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
  case ESP_SLEEP_WAKEUP_EXT0: return "EXT0 / USB or RTC";
  case ESP_SLEEP_WAKEUP_EXT1: return "EXT1 / BUTTON";
  case ESP_SLEEP_WAKEUP_TIMER: return "TIMER / MINUTE";
  case ESP_SLEEP_WAKEUP_TOUCHPAD: return "TOUCHPAD";
  case ESP_SLEEP_WAKEUP_ULP: return "ULP";
  default: return "COLD RESET / OTHER";
  }
}

const char *resetReasonName(esp_reset_reason_t reason) {
  switch (reason) {
  case ESP_RST_POWERON: return "POWER ON";
  case ESP_RST_EXT: return "EXTERNAL PIN";
  case ESP_RST_SW: return "SOFTWARE";
  case ESP_RST_PANIC: return "PANIC";
  case ESP_RST_INT_WDT: return "INT WATCHDOG";
  case ESP_RST_TASK_WDT: return "TASK WATCHDOG";
  case ESP_RST_WDT: return "WATCHDOG";
  case ESP_RST_DEEPSLEEP: return "DEEP SLEEP";
  case ESP_RST_BROWNOUT: return "BROWNOUT";
  default: return "OTHER";
  }
}

void drawReason(bool wake) {
  beginAppDisplay(wake ? "WAKE REASON" : "RESET REASON");
  useSmallText(8, 62);
  Watchy::display.setTextSize(2);
  if (wake) {
    Watchy::display.println(wakeReasonName(esp_sleep_get_wakeup_cause()));
  } else {
    Watchy::display.println(resetReasonName(esp_reset_reason()));
  }
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 130);
  Watchy::display.print("RAW CODE: ");
  Watchy::display.println(wake ? static_cast<int>(esp_sleep_get_wakeup_cause())
                               : static_cast<int>(esp_reset_reason()));
  finishAppDisplay();
}

void drawButtonCounts(const uint16_t counts[3]) {
  beginAppDisplay("BUTTON TESTER");
  useSmallText(12, 55);
  Watchy::display.print("MENU   "); Watchy::display.println(counts[0]);
  Watchy::display.setCursor(12, 85);
  Watchy::display.print("UP     "); Watchy::display.println(counts[1]);
  Watchy::display.setCursor(12, 115);
  Watchy::display.print("DOWN   "); Watchy::display.println(counts[2]);
  Watchy::display.setCursor(12, 165);
  Watchy::display.println("Press buttons. BACK exits.");
  finishAppDisplay();
}

void runButtonTester() {
  WatchyUi::Input::begin();
  uint16_t counts[3] = {};
  drawButtonCounts(counts);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) {
      return;
    }
    uint8_t index = event == WatchyUi::Event::UP
                        ? 1
                        : event == WatchyUi::Event::DOWN ? 2 : 0;
    if (event == WatchyUi::Event::SELECT || event == WatchyUi::Event::UP ||
        event == WatchyUi::Event::DOWN) {
      counts[index]++;
      drawButtonCounts(counts);
    }
  }
}

void drawVibration(uint8_t pattern) {
  const char *const names[] = {"TAP", "DOUBLE", "TRIPLE", "LONG", "HEARTBEAT"};
  beginAppDisplay("VIBRATION LAB");
  useSmallText(30, 70);
  Watchy::display.setTextSize(3);
  Watchy::display.println(names[pattern]);
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 145);
  Watchy::display.println("UP/DOWN: PATTERN SELECT: PLAY");
  Watchy::display.setCursor(8, 175);
  Watchy::display.println("BACK: EXIT");
  finishAppDisplay();
}

void playPattern(uint8_t pattern) {
  const uint8_t pulses[] = {1, 2, 3, 1, 2};
  for (uint8_t pulse = 0; pulse < pulses[pattern]; pulse++) {
    Watchy::vibMotor(
        pattern == 3 ? 800 : pattern == 4 && pulse == 1 ? 180 : 80, 1);
    if (pulse + 1 < pulses[pattern]) delay(pattern == 4 ? 130 : 100);
  }
}

void runVibrationLab() {
  WatchyUi::Input::begin();
  uint8_t pattern = 0;
  drawVibration(pattern);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      pattern = WatchyUi::ListView::previous(pattern, 5);
      drawVibration(pattern);
    } else if (event == WatchyUi::Event::DOWN) {
      pattern = WatchyUi::ListView::next(pattern, 5);
      drawVibration(pattern);
    } else if (event == WatchyUi::Event::SELECT) {
      playPattern(pattern);
    }
  }
}

void drawRuler() {
  beginAppDisplay("SCREEN RULER");
  uint16_t color = DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
  constexpr float pixelsPerMillimeter = 7.23f;
  Watchy::display.drawLine(0, 65, 199, 65, color);
  useSmallText(2, 48);
  Watchy::display.println("0         10        20   27mm");
  for (uint8_t millimeter = 0; millimeter <= 27; millimeter++) {
    int16_t x = static_cast<int16_t>(millimeter * pixelsPerMillimeter + 0.5f);
    Watchy::display.drawLine(x, 65, x, millimeter % 5 == 0 ? 83 : 75, color);
  }
  Watchy::display.drawLine(25, 105, 25, 199, color);
  for (uint8_t millimeter = 0; millimeter <= 13; millimeter++) {
    int16_t y = 105 + static_cast<int16_t>(millimeter * pixelsPerMillimeter + 0.5f);
    Watchy::display.drawLine(25, y, millimeter % 5 == 0 ? 45 : 35, y, color);
  }
  Watchy::display.setCursor(55, 125);
  Watchy::display.println("Approx. 7.23 px/mm");
  Watchy::display.setCursor(55, 145);
  Watchy::display.println("Verify against a ruler");
  finishAppDisplay();
}

void drawConverter(uint8_t tool, int value) {
  const char *title = tool == TEMPERATURE_CONVERTER ? "TEMPERATURE CONV" :
                      tool == LENGTH_CONVERTER ? "LENGTH CONVERTER" :
                      tool == WEIGHT_CONVERTER ? "WEIGHT CONVERTER" :
                      tool == BASE_CONVERTER ? "BASE CONVERTER" : "PACE CONVERTER";
  beginAppDisplay(title);
  useSmallText(5, 42);
  if (tool == TEMPERATURE_CONVERTER) {
    Watchy::display.print(value); Watchy::display.println(" C");
    Watchy::display.setCursor(5, 72); Watchy::display.print(value * 1.8f + 32.0f, 1); Watchy::display.println(" F");
    Watchy::display.setCursor(5, 102); Watchy::display.print(value + 273.15f, 2); Watchy::display.println(" K");
  } else if (tool == LENGTH_CONVERTER) {
    Watchy::display.print(value); Watchy::display.println(" m");
    Watchy::display.setCursor(5, 72); Watchy::display.print(value * 3.28084f, 2); Watchy::display.println(" ft");
    Watchy::display.setCursor(5, 102); Watchy::display.print(value * 1.09361f, 2); Watchy::display.println(" yd");
    Watchy::display.setCursor(5, 132); Watchy::display.print(value / 1000.0f, 3); Watchy::display.println(" km");
  } else if (tool == WEIGHT_CONVERTER) {
    Watchy::display.print(value); Watchy::display.println(" kg");
    Watchy::display.setCursor(5, 72); Watchy::display.print(value * 2.20462f, 2); Watchy::display.println(" lb");
    Watchy::display.setCursor(5, 102); Watchy::display.print(value * 35.274f, 1); Watchy::display.println(" oz");
  } else if (tool == BASE_CONVERTER) {
    Watchy::display.print("DEC "); Watchy::display.println(value);
    Watchy::display.setCursor(5, 72); Watchy::display.print("HEX 0x"); Watchy::display.println(value, HEX);
    Watchy::display.setCursor(5, 102); Watchy::display.print("OCT 0"); Watchy::display.println(value, OCT);
    Watchy::display.setCursor(5, 132); Watchy::display.print("BIN "); Watchy::display.println(value, BIN);
  } else {
    int secondsPerKm = value;
    float kmh = 3600.0f / secondsPerKm;
    float minutesPerMile = secondsPerKm * 1.609344f / 60.0f;
    Watchy::display.print("PACE "); Watchy::display.print(secondsPerKm / 60); Watchy::display.print(':');
    printTwoDigits(secondsPerKm % 60); Watchy::display.println(" /km");
    Watchy::display.setCursor(5, 72); Watchy::display.print(kmh, 2); Watchy::display.println(" km/h");
    Watchy::display.setCursor(5, 102); Watchy::display.print(minutesPerMile, 2); Watchy::display.println(" min/mile");
  }
  Watchy::display.setCursor(5, 175);
  Watchy::display.println("UP/DOWN: VALUE  BACK: EXIT");
  finishAppDisplay();
}

void runConverter(uint8_t tool) {
  WatchyUi::Input::begin();
  int value = tool == PACE_CONVERTER ? 300 : tool == BASE_CONVERTER ? 42 : 20;
  int step = tool == PACE_CONVERTER ? 5 : 1;
  drawConverter(tool, value);
  while (true) {
    WatchyUi::Event event = WatchyUi::Input::wait();
    if (event == WatchyUi::Event::BACK) return;
    if (event == WatchyUi::Event::UP) {
      value = min(tool == BASE_CONVERTER ? 65535 : tool == PACE_CONVERTER ? 1200 : 10000, value + step);
      drawConverter(tool, value);
    } else if (event == WatchyUi::Event::DOWN) {
      int minimum = tool == TEMPERATURE_CONVERTER ? -273 : tool == PACE_CONVERTER ? 60 : 0;
      value = max(minimum, value - step);
      drawConverter(tool, value);
    }
  }
}

} // namespace

void Watchy::showUtilityTool(uint8_t tool) {
  if (tool <= UUID_GENERATOR) {
    runGenerator(tool);
    showMenu(menuIndex, false);
    return;
  }
  switch (tool) {
  case I2C_SCANNER: drawI2cScanner(); return;
  case CHIP_INFO: drawChipInfo(); return;
  case HEAP_MONITOR: drawHeap(); return;
  case WAKE_REASON: drawReason(true); return;
  case RESET_CAUSE: drawReason(false); return;
  case BUTTON_TESTER: runButtonTester(); break;
  case VIBRATION_LAB: runVibrationLab(); break;
  case SCREEN_RULER: drawRuler(); return;
  case TEMPERATURE_CONVERTER:
  case LENGTH_CONVERTER:
  case WEIGHT_CONVERTER:
  case BASE_CONVERTER:
  case PACE_CONVERTER:
    runConverter(tool);
    break;
  default: return;
  }
  showMenu(menuIndex, false);
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
namespace WatchyDemo {
namespace {

void drawI2cPreview() {
  beginAppDisplay("I2C SCANNER");
  useSmallText(2, 34);
  Watchy::display.println("0x18 0x51");
  Watchy::display.setCursor(2, 145);
  Watchy::display.println("DEVICES FOUND: 2");
  Watchy::display.setCursor(2, 175);
  Watchy::display.println("Read-only address probe");
  finishAppDisplay();
}

void drawChipPreview() {
  beginAppDisplay("CHIP INFO");
  useSmallText(6, 42);
  Watchy::display.println("MODEL       ESP32-S3");
  Watchy::display.setCursor(6, 65);
  Watchy::display.println("CORES       2");
  Watchy::display.setCursor(6, 88);
  Watchy::display.println("REVISION    2");
  Watchy::display.setCursor(6, 111);
  Watchy::display.println("CPU MHz     40");
  Watchy::display.setCursor(6, 134);
  Watchy::display.println("FLASH       8 MB");
  Watchy::display.setCursor(6, 157);
  Watchy::display.println("SDK         ARDUINO ESP32");
  finishAppDisplay();
}

void drawHeapPreview() {
  beginAppDisplay("HEAP MONITOR");
  useSmallText(4, 46);
  Watchy::display.println("FREE HEAP   216384");
  Watchy::display.setCursor(4, 76);
  Watchy::display.println("MIN FREE    201728");
  Watchy::display.setCursor(4, 106);
  Watchy::display.println("MAX BLOCK   110592");
  Watchy::display.setCursor(4, 136);
  Watchy::display.println("HEAP SIZE   327680");
  Watchy::display.setCursor(4, 170);
  Watchy::display.println("Bytes; no PSRAM on Watchy v3");
  finishAppDisplay();
}

void drawReasonPreview(bool wake) {
  beginAppDisplay(wake ? "WAKE REASON" : "RESET REASON");
  useSmallText(8, 62);
  Watchy::display.setTextSize(2);
  Watchy::display.println(wake ? "TIMER / MINUTE" : "POWER ON");
  Watchy::display.setTextSize(1);
  Watchy::display.setCursor(8, 130);
  Watchy::display.print("RAW CODE: ");
  Watchy::display.println(wake ? 4 : 1);
  finishAppDisplay();
}

} // namespace

void renderUtilityPreview(uint8_t tool) {
  switch (tool) {
  case COIN_FLIP: drawGenerator(tool, "HEADS"); break;
  case D6_DICE: drawGenerator(tool, "4"); break;
  case D20_DICE: drawGenerator(tool, "17"); break;
  case RANDOM_NUMBER: drawGenerator(tool, "742"); break;
  case DECISION_MAKER: drawGenerator(tool, "YES"); break;
  case PASSWORD_GENERATOR: drawGenerator(tool, "N7vK4!pQ2xLm"); break;
  case UUID_GENERATOR:
    drawGenerator(tool, "2f7469e8-9f4a-4d73-b156-874dda019e62");
    break;
  case I2C_SCANNER: drawI2cPreview(); break;
  case CHIP_INFO: drawChipPreview(); break;
  case HEAP_MONITOR: drawHeapPreview(); break;
  case WAKE_REASON: drawReasonPreview(true); break;
  case RESET_CAUSE: drawReasonPreview(false); break;
  case BUTTON_TESTER: {
    const uint16_t counts[] = {3, 2, 4};
    drawButtonCounts(counts);
    break;
  }
  case VIBRATION_LAB: drawVibration(4); break;
  case SCREEN_RULER: drawRuler(); break;
  case TEMPERATURE_CONVERTER: drawConverter(tool, 20); break;
  case LENGTH_CONVERTER: drawConverter(tool, 20); break;
  case WEIGHT_CONVERTER: drawConverter(tool, 20); break;
  case BASE_CONVERTER: drawConverter(tool, 42); break;
  case PACE_CONVERTER: drawConverter(tool, 300); break;
  default: break;
  }
}

} // namespace WatchyDemo
#endif