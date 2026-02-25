#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include <map>
#include <vector>

RTC_DATA_ATTR static int g_mode = 0;

namespace {

void vibMorseChar(char c) {
  const int tiLength = 100;
  std::map<int, std::vector<int>> morseTable;
  morseTable[' '] = {0};
  morseTable['+'] = {0, 0, 0};
  morseTable['0'] = {3, 0, 3, 0, 3, 0, 3, 0, 3};
  morseTable['1'] = {1, 0, 3, 0, 3, 0, 3, 0, 3};
  morseTable['2'] = {1, 0, 1, 0, 3, 0, 3, 0, 3};
  morseTable['3'] = {1, 0, 1, 0, 1, 0, 3, 0, 3};
  morseTable['4'] = {1, 0, 1, 0, 1, 0, 1, 0, 3};
  morseTable['5'] = {1, 0, 1, 0, 1, 0, 1, 0, 1};
  morseTable['6'] = {3, 0, 1, 0, 1, 0, 1, 0, 1};
  morseTable['7'] = {3, 0, 3, 0, 1, 0, 1, 0, 1};
  morseTable['8'] = {3, 0, 3, 0, 3, 0, 1, 0, 1};
  morseTable['9'] = {3, 0, 3, 0, 3, 0, 3, 0, 1};

  pinMode(VIB_MOTOR_PIN, OUTPUT);
  for (int i : morseTable[c]) {
    if (i == 0) {
      delay(tiLength);
    } else {
      digitalWrite(VIB_MOTOR_PIN, true);
      delay(i * tiLength);
      digitalWrite(VIB_MOTOR_PIN, false);
    }
  }
}

void vibMorseString(const String &s) {
  unsigned int length = s.length();
  for (unsigned int i = 0; i < length; i++) {
    vibMorseChar(s.charAt(i));
    vibMorseChar('+');
  }
}

void morseTime(Watchy &watchy) {
  char time[6];
  snprintf(time, sizeof(time), "%02d %02d", watchy.currentTime.Hour, watchy.currentTime.Minute);
  vibMorseString(String(time));
}

} // namespace

void showWatchFace_BinaryBlocks(Watchy &watchy) {
  UiSDK::initScreen(watchy.display);
  const uint16_t themeBg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t themeFg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  uint16_t primaryColor = themeFg;
  uint16_t secondaryColor = themeBg;
  bool hands = true;

  if (g_mode == 0 || g_mode == 1) {
    primaryColor = themeFg;
    secondaryColor = themeBg;
  } else {
    primaryColor = themeBg;
    secondaryColor = themeFg;
  }
  hands = (g_mode == 0 || g_mode == 2);

  UiSDK::fillScreen(watchy.display, secondaryColor);

  UiSDK::fillRect(watchy.display, 16, 16, 170, 170, primaryColor);
  UiSDK::fillRect(watchy.display, 61, 61, 80, 80, secondaryColor);

  int hourD1 = watchy.currentTime.Hour / 10;
  int hourD2 = watchy.currentTime.Hour % 10;
  int minuteD1 = watchy.currentTime.Minute / 10;
  int minuteD2 = watchy.currentTime.Minute % 10;

  if (hourD1 == 1) {
    UiSDK::fillRect(watchy.display, 61, 109, 16, 16, primaryColor);
  }
  if (hourD1 == 2) {
    UiSDK::fillRect(watchy.display, 61, 77, 16, 16, primaryColor);
  }

  if (hourD2 / 8 >= 1) {
    UiSDK::fillRect(watchy.display, 125, 63, 16, 16, primaryColor);
  }
  if ((hourD2 % 8) / 4 >= 1) {
    UiSDK::fillRect(watchy.display, 125, 83, 16, 16, primaryColor);
  }
  if ((hourD2 % 4) / 2 >= 1) {
    UiSDK::fillRect(watchy.display, 125, 103, 16, 16, primaryColor);
  }
  if (hourD2 % 2 == 1) {
    UiSDK::fillRect(watchy.display, 125, 123, 16, 16, primaryColor);
  }

  if (minuteD1 / 4 >= 1) {
    UiSDK::fillRect(watchy.display, 16, 60, 16, 16, secondaryColor);
  }
  if ((minuteD1 % 4) / 2 >= 1) {
    UiSDK::fillRect(watchy.display, 16, 92, 16, 16, secondaryColor);
  }
  if (minuteD1 % 2 == 1) {
    UiSDK::fillRect(watchy.display, 16, 124, 16, 16, secondaryColor);
  }

  if (minuteD2 / 8 >= 1) {
    UiSDK::fillRect(watchy.display, 170, 44, 16, 16, secondaryColor);
  }
  if ((minuteD2 % 8) / 4 >= 1) {
    UiSDK::fillRect(watchy.display, 170, 76, 16, 16, secondaryColor);
  }
  if ((minuteD2 % 4) / 2 >= 1) {
    UiSDK::fillRect(watchy.display, 170, 108, 16, 16, secondaryColor);
  }
  if (minuteD2 % 2 == 1) {
    UiSDK::fillRect(watchy.display, 170, 140, 16, 16, secondaryColor);
  }

  if (hands) {
    const float deg2rad = 0.0174532925f;
    float hourAngle = deg2rad * (360.0f * ((float)watchy.currentTime.Hour + (float)watchy.currentTime.Minute / 60.0f) / 12.0f);
    float hourX = 100.0f + sin(hourAngle) * 47.0f / (float)max(abs(cos(hourAngle)), abs(sin(hourAngle)));
    float hourY = 100.0f - (cos(-hourAngle)) * (47.0f / (float)max(abs(cos(-hourAngle)), abs(sin(-hourAngle))));
    UiSDK::fillCircle(watchy.display, hourX, hourY, 7, secondaryColor);

    float minuteAngle = deg2rad * (360.0f * (float)watchy.currentTime.Minute / 60.0f);
    float minutesX = 100.0f + sin(minuteAngle) * 92.0f / (float)max(abs(cos(minuteAngle)), abs(sin(minuteAngle)));
    float minutesY = 100.0f - cos(-minuteAngle) * 92.0f / (float)max(abs(cos(-minuteAngle)), abs(sin(-minuteAngle)));
    UiSDK::fillCircle(watchy.display, minutesX, minutesY, 7, primaryColor);
  }
}

void handleBinaryBlocksButtonPress(Watchy &watchy) {
  if (guiState == WATCHFACE_STATE) {
    uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();

    if (wakeupBit & UP_BTN_MASK) {
      Watchy::RTC.read(watchy.currentTime);
      watchy.showWatchFace(true);
      morseTime(watchy);
      return;
    }
    if (wakeupBit & DOWN_BTN_MASK) {
      g_mode += 1;
      if (g_mode == 4) {
        g_mode = 0;
      }
      Watchy::RTC.read(watchy.currentTime);
      watchy.showWatchFace(true);
      return;
    }
    if (wakeupBit & MENU_BTN_MASK) {
      watchy.handleButtonPress();
      return;
    }
  } else {
    watchy.handleButtonPress();
  }
}
