#include "../../watchy/Watchy.h"
#include "../../sdk/UiSDK.h"
#include "Digdug/digdug.h"
#include <Fonts/FreeMonoBold9pt7b.h>

#define INDEX_SIZE 4

namespace {
  // 7-segment display definitions for Digdug digits
  const bool digdug_segs[10][7] = {
      {true, true, true, true, true, false, true},      // 0
      {false, false, true, true, false, false, false},  // 1
      {false, true, true, false, true, true, true},     // 2
      {false, false, true, true, true, true, true},     // 3
      {true, false, true, true, false, true, false},    // 4
      {true, false, false, true, true, true, true},     // 5
      {true, true, false, true, true, true, true},      // 6
      {false, false, true, true, true, false, false},   // 7
      {true, true, true, true, true, true, true},       // 8
      {true, false, true, true, true, true, true},      // 9
  };

  const int rock_indexs[8][2] = {
      {3, 7},
      {17, 11},
      {34, 7},
      {43, 10},
      {23, 34},
      {4, 41},
      {26, 41},
      {39, 40}};

  void drawBackground(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    // Ground
    UiSDK::drawBitmap(watchy.display, 0, INDEX_SIZE * 6, ground1, 200, INDEX_SIZE * 10, fgColor);
    UiSDK::drawBitmap(watchy.display, 0, INDEX_SIZE * 16, ground2, 200, INDEX_SIZE * 10, fgColor);
    UiSDK::drawBitmap(watchy.display, 0, INDEX_SIZE * 26, ground3, 200, INDEX_SIZE * 10, fgColor);
    UiSDK::drawBitmap(watchy.display, 0, INDEX_SIZE * 36, ground4, 200, INDEX_SIZE * 10, fgColor);

    // Center
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 23, INDEX_SIZE * 6, digdug_center, INDEX_SIZE * 4, INDEX_SIZE * 19, fgColor);

    // Player
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 23, INDEX_SIZE * 21, player, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);

    // Rock (placeholder - will draw in main function)
    // Title
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 1, INDEX_SIZE * 46, title, INDEX_SIZE * 12, INDEX_SIZE * 4, fgColor);
  }

  void drawRock(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    int choiced[3] = {-1, -1, -1};
    int randMax = 8;
    for (int cnt = 0; cnt < 3;) {
      int choice = random(randMax);
      int cnt2 = 0;
      for (; cnt2 < 3; cnt2++) {
        if (choiced[cnt2] == choice) {
          break;
        }
      }
      if (cnt2 != 3) {
        continue;
      }
      choiced[cnt] = choice;
      cnt++;
    }
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * rock_indexs[choiced[0]][0], INDEX_SIZE * rock_indexs[choiced[0]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * rock_indexs[choiced[1]][0], INDEX_SIZE * rock_indexs[choiced[1]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * rock_indexs[choiced[2]][0], INDEX_SIZE * rock_indexs[choiced[2]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
  }

  void draw7Seg(Watchy &watchy, const int &num, int index_x, int index_y) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    // row
    if (digdug_segs[num][0]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, fgColor);
    }

    if (digdug_segs[num][1]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, fgColor);
    }

    if (digdug_segs[num][2]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 6), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, fgColor);
    }

    if (digdug_segs[num][3]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 6), INDEX_SIZE * (index_y + 9), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, fgColor);
    }

    // column
    if (digdug_segs[num][4]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, fgColor);
    }

    if (digdug_segs[num][5]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, fgColor);
    }

    if (digdug_segs[num][6]) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 18), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, fgColor);
    }
  }

  void drawEnemy(Watchy &watchy, const int &num, int index_x, int index_y, int seed) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

    int choiced = -1;
    int randMax = 7;
    randomSeed(seed);
    for (;;) {
      int enemy_seg = random(randMax);
      if (digdug_segs[num][enemy_seg] == false) {
        continue;
      }
      choiced = enemy_seg;
      break;
    }

    int rand_x = random(7);
    int rand_y = random(10);

    const unsigned char *enemys[] = {waniF, pooka, pookaL, wani, waniL, pooka, pookaL, wani, waniL};
    int rand_enemy = random(9);
    int enemy_size_x = 4;
    int enemy_size_y = 4;
    if (rand_enemy == 0) {
      enemy_size_x = 10;
    }

    int draw_index_X = -1;
    int draw_index_Y = -1;
    switch (choiced) {
      case 0:
        draw_index_X = index_x;
        draw_index_Y = index_y + rand_y;
        break;
      case 1:
        draw_index_X = index_x;
        draw_index_Y = index_y + 9 + rand_y;
        break;
      case 2:
        draw_index_X = index_x + 6;
        draw_index_Y = index_y + rand_y;
        break;
      case 3:
        draw_index_X = index_x + 6;
        draw_index_Y = index_y + 9 + rand_y;
        break;
      case 4:
        draw_index_X = index_x + rand_x;
        draw_index_Y = index_y;
        break;
      case 5:
        draw_index_X = index_x + rand_x;
        draw_index_Y = index_y + 9;
        break;
      case 6:
        draw_index_X = index_x + rand_x;
        draw_index_Y = index_y + 18;
        break;
      default:
        break;
    }

    UiSDK::drawBitmap(watchy.display, INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, enemys[rand_enemy], INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, bgColor);
    if (rand_enemy == 0) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, waniFB, INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, fgColor);
    }
  }

  void drawBattery(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

    int8_t batteryLevel = 0;
    float VBAT = watchy.getBatteryVoltage();
    if (VBAT > 3.8) {
      batteryLevel = 3;
    } else if (VBAT > 3.4 && VBAT <= 3.8) {
      batteryLevel = 2;
    } else if (VBAT > 3.0 && VBAT <= 3.4) {
      batteryLevel = 1;
    } else if (VBAT <= 3.0) {
      batteryLevel = 0;
    }

    for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 46 - INDEX_SIZE * 4 * batterySegments, INDEX_SIZE * 46, player_rest, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
    }
  }

  void drawFlower(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    static bool NTP_SYNC = false;

    if (watchy.currentTime.Minute == 0) {
      NTP_SYNC = false;
      watchy.connectWiFi();
      if (WIFI_CONFIGURED) {
        NTP_SYNC = watchy.syncNTP();
      }
    }
    if (WIFI_CONFIGURED) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 46, INDEX_SIZE * 2, flower, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor); // wifi flower
    }
    if (NTP_SYNC) {
      UiSDK::drawBitmap(watchy.display, INDEX_SIZE * 42, INDEX_SIZE * 2, flower, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor); // ntp flower
    }
    WiFi.mode(WIFI_OFF);
    btStop();
  }

  void drawDate(Watchy &watchy) {
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
    UiSDK::setTextColor(watchy.display, fgColor);
    UiSDK::setFont(watchy.display, &FreeMonoBold9pt7b);

    String score = "1P";
    if (watchy.currentTime.Month < 10) {
      score += " ";
    }
    score += watchy.currentTime.Month;
    if (watchy.currentTime.Day < 10) {
      score += "0";
    }
    score += watchy.currentTime.Day;
    UiSDK::setCursor(watchy.display, INDEX_SIZE * 0, INDEX_SIZE * 4);
    UiSDK::print(watchy.display, score);

    String hiscore = " HIGH";
    hiscore += tmYearToCalendar(watchy.currentTime.Year);
    UiSDK::print(watchy.display, hiscore);
  }
}  // namespace

void showWatchFace_Digdug_Watch(Watchy &watchy) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  UiSDK::initScreen(watchy.display);

  drawBackground(watchy);
  // WifiFlower
  drawFlower(watchy);

  // Hour
  draw7Seg(watchy, watchy.currentTime.Hour / 10, 1, 17);
  draw7Seg(watchy, watchy.currentTime.Hour % 10, 12, 17);

  // Minute
  draw7Seg(watchy, watchy.currentTime.Minute / 10, 28, 17);
  draw7Seg(watchy, watchy.currentTime.Minute % 10, 39, 17);

  drawEnemy(watchy, watchy.currentTime.Hour / 10, 1, 17, watchy.currentTime.Minute + 3);
  drawEnemy(watchy, watchy.currentTime.Hour % 10, 12, 17, watchy.currentTime.Minute + 7);
  drawEnemy(watchy, watchy.currentTime.Minute / 10, 28, 17, watchy.currentTime.Minute + 11);
  drawEnemy(watchy, watchy.currentTime.Minute % 10, 39, 17, watchy.currentTime.Minute + 13);

  // Battery
  drawBattery(watchy);

  // Rock
  drawRock(watchy);

  // Date
  drawDate(watchy);
}
