#include "Watchy_Skykid.h"
#include "Skykid.h"
#include "FreeSansBold6pt7b.h"

bool isDaytime = true;
bool isLand = true;

#ifdef WATCHY_SIM
bool NTP_SYNC;
#else
RTC_DATA_ATTR bool NTP_SYNC;
#endif

void WatchySkykid::drawWatchFace(){
    if (currentTime.Hour < 6 || currentTime.Hour >= 18) {
        isDaytime = false;
    }
    else {
        isDaytime = true;
    }

#ifdef WATCHY_SIM
    srand(currentTime.Hour + currentTime.Minute + currentTime.Wday+ 37);
    int randNum = rand() % 10;
#else
    randomSeed(currentTime.Hour + currentTime.Minute + currentTime.Wday + 37);
    int randNum = random(10);
#endif
    if (randNum % 2 == 0) {
        isLand = false;
    }
    else {
        isLand = true;
    }

    display.fillScreen(isDaytime ? GxEPD_WHITE : GxEPD_BLACK);

    //Backgroud
    drawBackground();

    //Hour
    int hour = 0;
    if (currentTime.Hour >= 12) {
        hour = currentTime.Hour - 12;
    }
    else {
        hour = currentTime.Hour;
    }
    if (hour >= 10) {
        drawSeg(hour / 10, 1, 6, true);
    }
    drawSeg(hour % 10, 6, 6);

    //Minute
    drawSeg(currentTime.Minute / 10, 18, 9);
    drawSeg(currentTime.Minute % 10, 28, 9);

    //Battery
    drawBattery();

    //Date
    drawDate();

    // Player
    int playerLocate = 0;
    if(randNum<3){
      playerLocate = 0;
    }else if (randNum<6){
      playerLocate = 1;
    }else{
      playerLocate = 2;
    }
    if (isDaytime) {
        display.drawBitmap(INDEX_SIZE * (3 + 5 * playerLocate), INDEX_SIZE * 21, player_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
    }
    display.drawBitmap(INDEX_SIZE * (3 + 5 * playerLocate), INDEX_SIZE * 21, player, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);

    // bomb
    drawBomb(playerLocate);

}

void WatchySkykid::drawBackground() {
    //Title
    display.drawBitmap(0, 0, title, INDEX_SIZE * 19, INDEX_SIZE * 4, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);

    //Land or Sea
    display.drawBitmap(0, INDEX_SIZE * 44, isLand ? land : sea, 200, INDEX_SIZE * 6, isDaytime ? GxEPD_BLACK : GxEPD_WHITE );

    //Sun or Moon
    display.drawBitmap(INDEX_SIZE * 40, INDEX_SIZE * 5, isDaytime ? sun : moon, INDEX_SIZE * 8, INDEX_SIZE * 8, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);

    // AirEnemy
    drawAirEnemy();

    // LandEnemy
    drawLandEnemy();

    // target
    drawTarget();

    // Vinus
    display.drawBitmap(INDEX_SIZE * 40, INDEX_SIZE * 30, vinus1, INDEX_SIZE * 8, INDEX_SIZE * 16, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);

    // NTP
    drawFire();
}

const int air_enemy_indexs[6][2] = {
    {18,26},
    {24,23},
    {27,33},
    {33,30},
    {35,22},
    {42,18}
};

void WatchySkykid::drawAirEnemy()
{
    int randMax = 6;
    const int enemyNum = 3;
    int choiced[enemyNum] = { -1, -1, -1 };

    for (int cnt = 0; cnt < enemyNum; ) {
#ifdef WATCHY_SIM
        int choice = rand()% randMax;
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
        if (!isDaytime) {
            display.drawBitmap(INDEX_SIZE * air_enemy_indexs[choiced[cnt]][0], INDEX_SIZE * air_enemy_indexs[choiced[cnt]][1], air_enemy_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);
        }

        display.drawBitmap(INDEX_SIZE * air_enemy_indexs[choiced[cnt]][0], INDEX_SIZE * air_enemy_indexs[choiced[cnt]][1], air_enemy, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
    }
}

const int land_enemy_indexs[4][2] = {
    {20,42},
    {25,42},
    {30,42},
    {35,42}
};

void WatchySkykid::drawLandEnemy()
{
    int choiced[2] = { -1, -1 };
    int randMax = 4;
    for (int cnt = 0; cnt < 2; ) {
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
    int choicedEnemy[2] = { -1, -1 };
    for (int cnt = 0; cnt < 2; cnt++) {
#ifdef WATCHY_SIM
        choicedEnemy[cnt] = rand() % randMax;
#else
        choicedEnemy[cnt] = random(randMax);
#endif
    }

    if (isLand) {
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[0]][0], INDEX_SIZE * land_enemy_indexs[choiced[0]][1], land_enemy_mask[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, isDaytime ? GxEPD_WHITE : GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[1]][0], INDEX_SIZE * land_enemy_indexs[choiced[1]][1], land_enemy_mask[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, isDaytime ? GxEPD_WHITE : GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE* land_enemy_indexs[choiced[0]][0], INDEX_SIZE* land_enemy_indexs[choiced[0]][1], land_enemy[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);
        display.drawBitmap(INDEX_SIZE* land_enemy_indexs[choiced[1]][0], INDEX_SIZE* land_enemy_indexs[choiced[1]][1], land_enemy[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);
    }
    else {
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[0]][0], INDEX_SIZE * land_enemy_indexs[choiced[0]][1], sea_enemy_mask[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[1]][0], INDEX_SIZE * land_enemy_indexs[choiced[1]][1], sea_enemy_mask[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[0]][0], INDEX_SIZE * land_enemy_indexs[choiced[0]][1], sea_enemy[choicedEnemy[0]], INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);
        display.drawBitmap(INDEX_SIZE * land_enemy_indexs[choiced[1]][0], INDEX_SIZE * land_enemy_indexs[choiced[1]][1], sea_enemy[choicedEnemy[1]], INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);
    }
}

void WatchySkykid::drawTarget()
{
#ifdef WATCHY_SIM
    int choice = rand() % 3;
#else
    int choice = random(3);
#endif
    if (isLand) {
        display.drawBitmap(INDEX_SIZE * 2, INDEX_SIZE * 38, land_target_mask[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, isDaytime ? GxEPD_WHITE : GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * 2, INDEX_SIZE * 38, land_target[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);
    }
    else {
        display.drawBitmap(INDEX_SIZE * 2, INDEX_SIZE * 38, sea_target_mask[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * 2, INDEX_SIZE * 38, sea_target[choice], INDEX_SIZE * 16, INDEX_SIZE * 8, GxEPD_WHITE);
    }
}

void WatchySkykid::drawFire()
{
    if (currentTime.Minute == 0) {
        NTP_SYNC = false;
#ifndef WATCHY_SIM
        connectWiFi();
#endif
        if (WIFI_CONFIGURED) {
#ifdef WATCHY_SIM
            NTP_SYNC = true;
#else
            NTP_SYNC = syncNTP();
#endif
        }
    }
    if (NTP_SYNC) {
        display.drawBitmap(INDEX_SIZE * 41, INDEX_SIZE * 29, vinus_fire, INDEX_SIZE * 3, INDEX_SIZE * 3, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); //ntp fire
    }
#ifndef WATCHY_SIM
    WiFi.mode(WIFI_OFF);
    btStop();
#endif
}


void WatchySkykid::drawDate() {
    display.setTextColor(isDaytime ? GxEPD_BLACK : GxEPD_WHITE);
    display.setFont(&FreeSansBold6pt7b);

    String score = "BARON ";
    if (currentTime.Month < 10) {
        score += " ";
    }
    score += currentTime.Month;
    if (currentTime.Day < 10) {
        score += "0";
    }
    score += currentTime.Day;
    display.setCursor(INDEX_SIZE * 19, INDEX_SIZE * 3);
    display.print(score);
}

void WatchySkykid::drawBattery(){
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if(VBAT > 3.8){
        batteryLevel = 3;
    }
    else if(VBAT > 3.4 && VBAT <= 3.8){
        batteryLevel = 2;
    }
    else if(VBAT > 3.0 && VBAT <= 3.4){
        batteryLevel = 1;
    }
    else if(VBAT <= 3.0){
        batteryLevel = 0;
    }

    for(int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++){
        if (isDaytime) {
            display.drawBitmap(INDEX_SIZE * 46 - INDEX_SIZE * 4 * batterySegments, INDEX_SIZE * 0, player_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); //mask
        }
        display.drawBitmap(INDEX_SIZE * 46 - INDEX_SIZE * 4 *batterySegments, INDEX_SIZE * 0, player, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); //player

    }
}

void WatchySkykid::drawSeg(const int& num, int index_x, int index_y, bool one_left)
{
    int disp_size_x = 10;
    if (num == 1) {
        disp_size_x = 5;
        if (one_left) {
            index_x += 0; // left side
        }
        else {
            index_x += 5; // right side
        }
    }

    display.drawBitmap(INDEX_SIZE * index_x, INDEX_SIZE * index_y, num_allArray[num], INDEX_SIZE * disp_size_x, INDEX_SIZE * 12, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);

    if (isDaytime) {
        display.drawBitmap(INDEX_SIZE * index_x, INDEX_SIZE * index_y, num_mask_allArray[num], INDEX_SIZE * disp_size_x, INDEX_SIZE * 12, GxEPD_WHITE);
    }
}

void WatchySkykid::drawBomb(const int &playerLocate) {
    const int bomb_indexs[3][2] = {
        {4,25},
        {4,29},
        {4,33},
    };

#ifdef WATCHY_SIM
    for (int cnt = 0; cnt < 3; cnt++) {
        display.drawBitmap(INDEX_SIZE * (bomb_indexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bomb_indexs[cnt][1], bomb, INDEX_SIZE * 2, INDEX_SIZE * 4, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); // bomb draw
        drawBullet(cnt, true);
        Sleep(500);
        display.drawBitmap(INDEX_SIZE * (bomb_indexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bomb_indexs[cnt][1], bomb, INDEX_SIZE * 2, INDEX_SIZE * 4, isDaytime ? GxEPD_WHITE : GxEPD_BLACK); // erase
        drawBullet(cnt, false);
    }
#else
    display.display(true);
    for (int cnt = 0; cnt < 3; cnt++) {
        display.drawBitmap(INDEX_SIZE * (bomb_indexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bomb_indexs[cnt][1], bomb, INDEX_SIZE * 2, INDEX_SIZE * 4, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); // draw
        drawBullet(cnt, true);
        display.display(true);
        delay(200);
        display.drawBitmap(INDEX_SIZE * (bomb_indexs[cnt][0] + playerLocate * 5), INDEX_SIZE * bomb_indexs[cnt][1], bomb, INDEX_SIZE * 2, INDEX_SIZE * 4, isDaytime ? GxEPD_WHITE : GxEPD_BLACK); // erase
        drawBullet(cnt, false);
        display.display(true);
    }
#endif

    switch (playerLocate) {
    case 0:
        display.drawBitmap(INDEX_SIZE * 3, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 3, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // exporde
#ifdef WATCHY_SIM
        Sleep(200);
#else
        display.display(true);
        delay(200);
#endif
        display.drawBitmap(INDEX_SIZE * 3, INDEX_SIZE * 35, five_hundred_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 3, INDEX_SIZE * 35, five_hundred, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // 500
        break;
    case 1:
        display.drawBitmap(INDEX_SIZE * 8, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 8, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // exporde
#ifdef WATCHY_SIM
        Sleep(200);
#else
        display.display(true);
        delay(200);
#endif
        display.drawBitmap(INDEX_SIZE * 8, INDEX_SIZE * 35, thousand_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 8, INDEX_SIZE * 35, thousand, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // 1000
#ifdef WATCHY_SIM
        Sleep(200);
#else
        display.display(true);
        delay(200);
#endif
        display.drawBitmap(INDEX_SIZE * 12, INDEX_SIZE * 32, medal_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 12, INDEX_SIZE * 32, medal, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // medal

        // Vinus
        display.drawBitmap(INDEX_SIZE * 40, INDEX_SIZE * 30, vinus1, INDEX_SIZE * 8, INDEX_SIZE * 16, isDaytime ? GxEPD_WHITE : GxEPD_BLACK);
        display.drawBitmap(INDEX_SIZE * 40, INDEX_SIZE * 30, vinus2, INDEX_SIZE * 8, INDEX_SIZE * 16, isDaytime ? GxEPD_BLACK : GxEPD_WHITE);

        break;
    case 2:
        display.drawBitmap(INDEX_SIZE * 13, INDEX_SIZE * 40, exporde_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 13, INDEX_SIZE * 40, exporde, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // exporde
#ifdef WATCHY_SIM
        Sleep(200);
#else
        display.display(true);
        delay(200);
#endif
        display.drawBitmap(INDEX_SIZE * 13, INDEX_SIZE * 35, five_hundred_mask, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE); // mask
        display.drawBitmap(INDEX_SIZE * 13, INDEX_SIZE * 35, five_hundred, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); // 500
        break;
    default:
        break;
    }
}

void WatchySkykid::drawBullet(const int& seedNum, const bool& draw)
{
    const int bullet_indexs[3][3][2] = {
        {
        {19,36},    // 3
        {22,31},    // 6
        {17,31},    // 2
        },
        {
        {19,36},    // 3
        {24,34},    // 4
        {26,28},    // 7
        },
        {
        {30,29},    // 5
        {21,23},    // 8
        {17,31},    // 2
        },
    };

    if(draw){
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][0][0], INDEX_SIZE * bullet_indexs[seedNum][0][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); // draw
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][1][0], INDEX_SIZE * bullet_indexs[seedNum][1][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); // draw
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][2][0], INDEX_SIZE * bullet_indexs[seedNum][2][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_BLACK : GxEPD_WHITE); // draw
    }
    else {
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][0][0], INDEX_SIZE * bullet_indexs[seedNum][0][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_WHITE : GxEPD_BLACK); // erase
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][1][0], INDEX_SIZE * bullet_indexs[seedNum][1][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_WHITE : GxEPD_BLACK); // erase
        display.drawBitmap(INDEX_SIZE * bullet_indexs[seedNum][2][0], INDEX_SIZE * bullet_indexs[seedNum][2][1], bullet, INDEX_SIZE * 2, INDEX_SIZE * 2, isDaytime ? GxEPD_WHITE : GxEPD_BLACK); // erase
    }

}