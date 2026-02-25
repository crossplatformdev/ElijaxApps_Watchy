#include "../../../watchy/Watchy.h"
#include "../../../sdk/UiSDK.h"
#include "FreeSansBold6pt7b.h"
#include "Skykid.h"

#ifndef INDEX_SIZE
#define INDEX_SIZE 4
#endif

namespace {

Watchy *g_watchy = nullptr;
bool g_isDaytime = true;
bool g_isLand = true;

#ifdef WATCHY_SIM
static bool g_ntpSync;
#else
RTC_DATA_ATTR static bool g_ntpSync;
#endif

void drawAirEnemy();
void drawLandEnemy();
void drawTarget();
void drawFire();
void drawDate();
void drawBattery();
void drawSeg(const int &num, int indexX, int indexY, bool oneLeft = false);
void drawBomb(const int &playerLocate);
void drawBullet(const int &seedNum, const bool &draw);

void drawBackground() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::drawBitmap(g_watchy->display, 0, 0, title, INDEX_SIZE * 19, INDEX_SIZE * 4, fgColor);
  UiSDK::drawBitmap(g_watchy->display, 0, INDEX_SIZE * 44, g_isLand ? land : sea, 200, INDEX_SIZE * 6, fgColor);
  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 40, INDEX_SIZE * 5, g_isDaytime ? sun : moon, INDEX_SIZE * 8, INDEX_SIZE * 8, fgColor);

  drawAirEnemy();
  drawLandEnemy();
  drawTarget();

  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 40, INDEX_SIZE * 30, vinus1, INDEX_SIZE * 8, INDEX_SIZE * 16, fgColor);

  drawFire();
}

const int kAirEnemyIndexs[6][2] = {
    {18, 26},
    {24, 23},
    {27, 33},
    {33, 30},
    {35, 22},
    {42, 18},
};

void drawAirEnemy() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  int randMax = 6;
  const int enemyNum = 3;
  int choiced[enemyNum] = {-1, -1, -1};

  for (int cnt = 0; cnt < enemyNum;) {
#ifdef WATCHY_SIM
    int choice = rand() % randMax;
#else
    int choice = random(randMax);
#endif
    int cnt2 = 0;
    for (; cnt2 < enemyNum; cnt2++) {
      if (choiced[cnt2] == choice) {
        break;
      }
    }
    if (cnt2 != enemyNum) {
      continue;
    }
    choiced[cnt] = choice;
    cnt++;
  }

  for (int cnt = 0; cnt < enemyNum; cnt++) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kAirEnemyIndexs[choiced[cnt]][0], INDEX_SIZE * kAirEnemyIndexs[choiced[cnt]][1], air_enemy_mask,
                                 INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kAirEnemyIndexs[choiced[cnt]][0], INDEX_SIZE * kAirEnemyIndexs[choiced[cnt]][1], air_enemy,
                                 INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
  }
}

const int kLandEnemyIndexs[4][2] = {
    {20, 42},
    {25, 42},
    {30, 42},
    {35, 42},
};

void drawLandEnemy() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  int choiced[2] = {-1, -1};
  int randMax = 4;
  for (int cnt = 0; cnt < 2;) {
#ifdef WATCHY_SIM
    int choice = rand() % randMax;
#else
    int choice = random(randMax);
#endif
    int cnt2 = 0;
    for (; cnt2 < 2; cnt2++) {
      if (choiced[cnt2] == choice) {
        break;
      }
    }
    if (cnt2 != 2) {
      continue;
    }
    choiced[cnt] = choice;

    cnt++;
  }

  randMax = 2;
  int choicedEnemy[2] = {-1, -1};
  for (int cnt = 0; cnt < 2; cnt++) {
#ifdef WATCHY_SIM
    choicedEnemy[cnt] = rand() % randMax;
#else
    choicedEnemy[cnt] = random(randMax);
#endif
  }

  if (g_isLand) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[0]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[0]][1],
                                 land_enemy_mask[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[1]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[1]][1],
                                 land_enemy_mask[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[0]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[0]][1],
                                 land_enemy[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[1]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[1]][1],
                                 land_enemy[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
  } else {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[0]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[0]][1],
                                 sea_enemy_mask[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[1]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[1]][1],
                                 sea_enemy_mask[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[0]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[0]][1],
                                 sea_enemy[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * kLandEnemyIndexs[choiced[1]][0], INDEX_SIZE * kLandEnemyIndexs[choiced[1]][1],
                                 sea_enemy[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
  }
}

void drawTarget() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

#ifdef WATCHY_SIM
  int choice = rand() % 3;
#else
  int choice = random(3);
#endif
  if (g_isLand) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 2, INDEX_SIZE * 38, land_target_mask[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 2, INDEX_SIZE * 38, land_target[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, fgColor);
  } else {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 2, INDEX_SIZE * 38, sea_target_mask[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 2, INDEX_SIZE * 38, sea_target[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, fgColor);
  }
}

void drawFire() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  if (g_watchy->currentTime.Minute == 0) {
    g_ntpSync = false;
#ifndef WATCHY_SIM
    g_watchy->connectWiFi();
#endif
    if (WIFI_CONFIGURED) {
#ifdef WATCHY_SIM
      g_ntpSync = true;
#else
      g_ntpSync = g_watchy->syncNTP();
#endif
    }
  }
  if (g_ntpSync) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 41, INDEX_SIZE * 29, vinus_fire, INDEX_SIZE * 3, INDEX_SIZE * 3, fgColor);
  }
#ifndef WATCHY_SIM
  WiFi.mode(WIFI_OFF);
  btStop();
#endif
}

void drawDate() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

  UiSDK::setTextColor(g_watchy->display, fgColor);
  UiSDK::setFont(g_watchy->display, &FreeSansBold6pt7b);

  String score = "BARON ";
  if (g_watchy->currentTime.Month < 10) {
    score += " ";
  }
  score += g_watchy->currentTime.Month;
  if (g_watchy->currentTime.Day < 10) {
    score += "0";
  }
  score += g_watchy->currentTime.Day;
  UiSDK::setCursor(g_watchy->display, INDEX_SIZE * 19, INDEX_SIZE * 3);
  UiSDK::print(g_watchy->display, score);
}

void drawBattery() {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  int8_t batteryLevel = 0;
  float vbat = g_watchy->getBatteryVoltage();
  if (vbat > 3.8f) {
    batteryLevel = 3;
  } else if (vbat > 3.4f && vbat <= 3.8f) {
    batteryLevel = 2;
  } else if (vbat > 3.0f && vbat <= 3.4f) {
    batteryLevel = 1;
  } else if (vbat <= 3.0f) {
    batteryLevel = 0;
  }

  for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 46 - INDEX_SIZE * 4 * batterySegments, INDEX_SIZE * 0, player_mask, INDEX_SIZE * 4,
                                 INDEX_SIZE * 4, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 46 - INDEX_SIZE * 4 * batterySegments, INDEX_SIZE * 0, player, INDEX_SIZE * 4, INDEX_SIZE * 4,
                                 fgColor);
  }
}

void drawSeg(const int &num, int indexX, int indexY, bool oneLeft) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  int dispSizeX = 10;
  if (num == 1) {
    dispSizeX = 5;
    if (!oneLeft) {
      indexX += 5;
    }
  }

  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * indexX, INDEX_SIZE * indexY, num_allArray[num], INDEX_SIZE * dispSizeX, INDEX_SIZE * 12, fgColor);
  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * indexX, INDEX_SIZE * indexY, num_mask_allArray[num], INDEX_SIZE * dispSizeX, INDEX_SIZE * 12, bgColor);
}

void drawBomb(const int &playerLocate) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  const int bombIndexs[3][2] = {
      {4, 25},
      {4, 29},
      {4, 33},
  };

#ifdef WATCHY_SIM
  for (int cnt = 0; cnt < 3; cnt++) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (bombIndexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bombIndexs[cnt][1], bomb, INDEX_SIZE * 2,
                                 INDEX_SIZE * 4, fgColor);
    drawBullet(cnt, true);
    Sleep(500);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (bombIndexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bombIndexs[cnt][1], bomb, INDEX_SIZE * 2,
                                 INDEX_SIZE * 4, bgColor);
    drawBullet(cnt, false);
  }
#else
  UiSDK::displayUpdate(g_watchy->display, true);
  for (int cnt = 0; cnt < 3; cnt++) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (bombIndexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bombIndexs[cnt][1], bomb, INDEX_SIZE * 2,
                                 INDEX_SIZE * 4, fgColor);
    drawBullet(cnt, true);
    UiSDK::displayUpdate(g_watchy->display, true);
    delay(200);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (bombIndexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bombIndexs[cnt][1], bomb, INDEX_SIZE * 2,
                                 INDEX_SIZE * 4, bgColor);
    drawBullet(cnt, false);
    UiSDK::displayUpdate(g_watchy->display, true);
  }
#endif

  switch (playerLocate) {
    case 0:
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 3, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 3, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
#ifdef WATCHY_SIM
      Sleep(200);
#else
      UiSDK::displayUpdate(g_watchy->display, true);
      delay(200);
#endif
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 3, INDEX_SIZE * 35, five_hundred_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 3, INDEX_SIZE * 35, five_hundred, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
      break;
    case 1:
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 8, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 8, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
#ifdef WATCHY_SIM
      Sleep(200);
#else
      UiSDK::displayUpdate(g_watchy->display, true);
      delay(200);
#endif
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 8, INDEX_SIZE * 35, thousand_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 8, INDEX_SIZE * 35, thousand, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
#ifdef WATCHY_SIM
      Sleep(200);
#else
      UiSDK::displayUpdate(g_watchy->display, true);
      delay(200);
#endif
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 12, INDEX_SIZE * 32, medal_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 12, INDEX_SIZE * 32, medal, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);

      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 40, INDEX_SIZE * 30, vinus1, INDEX_SIZE * 8, INDEX_SIZE * 16, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 40, INDEX_SIZE * 30, vinus2, INDEX_SIZE * 8, INDEX_SIZE * 16, fgColor);
      break;
    case 2:
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 13, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 13, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
#ifdef WATCHY_SIM
      Sleep(200);
#else
      UiSDK::displayUpdate(g_watchy->display, true);
      delay(200);
#endif
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 13, INDEX_SIZE * 35, five_hundred_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, bgColor);
      UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * 13, INDEX_SIZE * 35, five_hundred, INDEX_SIZE * 4, INDEX_SIZE * 4, fgColor);
      break;
    default:
      break;
  }
}

void drawBullet(const int &seedNum, const bool &draw) {
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  const int bulletIndexs[3][3][2] = {
      {
          {19, 36},
          {22, 31},
          {17, 31},
      },
      {
          {19, 36},
          {24, 34},
          {26, 28},
      },
      {
          {30, 29},
          {21, 23},
          {17, 31},
      },
  };

  if (draw) {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][0][0], INDEX_SIZE * bulletIndexs[seedNum][0][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, fgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][1][0], INDEX_SIZE * bulletIndexs[seedNum][1][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, fgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][2][0], INDEX_SIZE * bulletIndexs[seedNum][2][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, fgColor);
  } else {
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][0][0], INDEX_SIZE * bulletIndexs[seedNum][0][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][1][0], INDEX_SIZE * bulletIndexs[seedNum][1][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, bgColor);
    UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * bulletIndexs[seedNum][2][0], INDEX_SIZE * bulletIndexs[seedNum][2][1], bullet, INDEX_SIZE * 2,
                                 INDEX_SIZE * 2, bgColor);
  }
}

} // namespace

void showWatchFace_Skykid(Watchy &watchy) {
  g_watchy = &watchy;

  if (watchy.currentTime.Hour < 6 || watchy.currentTime.Hour >= 18) {
    g_isDaytime = false;
  } else {
    g_isDaytime = true;
  }

#ifdef WATCHY_SIM
  srand(watchy.currentTime.Hour + watchy.currentTime.Minute + watchy.currentTime.Wday + 37);
  int randNum = rand() % 10;
#else
  randomSeed(watchy.currentTime.Hour + watchy.currentTime.Minute + watchy.currentTime.Wday + 37);
  int randNum = random(10);
#endif
  g_isLand = (randNum % 2 != 0);

  UiSDK::initScreen(watchy.display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);
  (void)fgColor;
  (void)bgColor;

  drawBackground();

  int hour = 0;
  if (watchy.currentTime.Hour >= 12) {
    hour = watchy.currentTime.Hour - 12;
  } else {
    hour = watchy.currentTime.Hour;
  }
  if (hour >= 10) {
    drawSeg(hour / 10, 1, 6, true);
  }
  drawSeg(hour % 10, 6, 6);

  drawSeg(watchy.currentTime.Minute / 10, 18, 9);
  drawSeg(watchy.currentTime.Minute % 10, 28, 9);

  drawBattery();
  drawDate();

  int playerLocate = 0;
  if (randNum < 3) {
    playerLocate = 0;
  } else if (randNum < 6) {
    playerLocate = 1;
  } else {
    playerLocate = 2;
  }
  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (3 + 5 * playerLocate), INDEX_SIZE * 21, player_mask, INDEX_SIZE * 4, INDEX_SIZE * 4,
                               UiSDK::getWatchfaceBg(BASE_POLARITY));
  UiSDK::drawBitmap(g_watchy->display, INDEX_SIZE * (3 + 5 * playerLocate), INDEX_SIZE * 21, player, INDEX_SIZE * 4, INDEX_SIZE * 4,
                               UiSDK::getWatchfaceFg(BASE_POLARITY));

  drawBomb(playerLocate);
}
