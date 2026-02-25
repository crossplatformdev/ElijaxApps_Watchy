#include "Watchy_Digdug.h"

#ifdef WATCHY_SIM
#include "../../FreeMonoBold9pt7b.h"
#else
#include <Fonts/FreeMonoBold9pt7b.h>
#endif

#ifdef WATCHY_SIM
bool NTP_SYNC;
#else
RTC_DATA_ATTR bool NTP_SYNC;
#endif
void WatchyDigdug::drawWatchFace(){
    display.fillScreen(GxEPD_WHITE);

    drawBackground();
    //WifiFlower
    drawFlower();

    //Hour
    draw7Seg(currentTime.Hour / 10, 1, 17);
    draw7Seg(currentTime.Hour % 10, 12, 17);

    //Minute
    draw7Seg(currentTime.Minute / 10, 28, 17);
    draw7Seg(currentTime.Minute % 10, 39, 17);

    drawEnemy(currentTime.Hour / 10, 1, 17, currentTime.Minute+3);
    drawEnemy(currentTime.Hour % 10, 12, 17, currentTime.Minute+7);
    drawEnemy(currentTime.Minute / 10, 28, 17, currentTime.Minute+11);
    drawEnemy(currentTime.Minute % 10, 39, 17, currentTime.Minute+13);

    //Battery
    drawBattery();

    //Date
    drawDate();

}

void WatchyDigdug::drawBackground() {
    //Ground
    display.drawBitmap(0, INDEX_SIZE * 6, ground1, 200, INDEX_SIZE * 10, GxEPD_BLACK);
    display.drawBitmap(0, INDEX_SIZE * 16, ground2, 200, INDEX_SIZE * 10, GxEPD_BLACK);
    display.drawBitmap(0, INDEX_SIZE * 26, ground3, 200, INDEX_SIZE * 10, GxEPD_BLACK);
    display.drawBitmap(0, INDEX_SIZE * 36, ground4, 200, INDEX_SIZE * 10, GxEPD_BLACK);

    //Center
    display.drawBitmap(INDEX_SIZE * 23, INDEX_SIZE * 6, digdug_center, INDEX_SIZE * 4, INDEX_SIZE * 19, GxEPD_BLACK);

    //Player
    display.drawBitmap(INDEX_SIZE * 23, INDEX_SIZE * 21, player, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_WHITE);

    //Rock
    drawRock();

    //Title
    display.drawBitmap(INDEX_SIZE * 1, INDEX_SIZE * 46, title, INDEX_SIZE * 12, INDEX_SIZE * 4, GxEPD_BLACK);
}

const bool digdug_segs[10][7] = {
    {true, true, true, true, true, false,true },     // 0
    {false,false,true, true, false,false,false},     // 1
    {false,true, true, false,true, true, true },     // 2
    {false,false,true, true, true, true, true },     // 3
    {true, false,true, true, false,true, false},     // 4
    {true, false,false,true, true, true, true },     // 5
    {true, true, false,true, true, true, true },     // 6
    {false,false,true, true, true, false,false},     // 7
    {true, true, true, true, true, true, true },     // 8
    {true, false,true, true, true, true, true },     // 9
};
void WatchyDigdug::draw7Seg(const int& num, int index_x, int index_y)
{
    // row
    if (digdug_segs[num][0]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, GxEPD_BLACK);
    }

    if (digdug_segs[num][1]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, GxEPD_BLACK);
    }

    if (digdug_segs[num][2]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 6), INDEX_SIZE * (index_y + 0), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, GxEPD_BLACK);
    }

    if (digdug_segs[num][3]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 6), INDEX_SIZE * (index_y + 9), seg_row, INDEX_SIZE * 4, INDEX_SIZE * 13, GxEPD_BLACK);
    }

    // column
    if (digdug_segs[num][4]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 0), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, GxEPD_BLACK);
    }

    if (digdug_segs[num][5]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 9), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, GxEPD_BLACK);
    }

    if (digdug_segs[num][6]) {
        display.drawBitmap(INDEX_SIZE * (index_x + 0), INDEX_SIZE * (index_y + 18), seg_column, INDEX_SIZE * 10, INDEX_SIZE * 4, GxEPD_BLACK);
    }

}

void WatchyDigdug::drawEnemy(const int& num, int index_x, int index_y, int seed)
{
    // enemy
    int choiced = -1;
    int randMax = 7;

#ifdef WATCHY_SIM
    srand(seed);
#else
    randomSeed(seed);
#endif
    for (;;) {
#ifdef WATCHY_SIM
        int enemy_seg = rand() % randMax;
#else
        int enemy_seg = random(randMax);
#endif
        if (digdug_segs[num][enemy_seg] == false) {
            continue;
        }
        choiced = enemy_seg;
        break;
    }

#ifdef WATCHY_SIM
    int rand_x = rand() % 7;
    int rand_y = rand() % 10;
#else
    int rand_x = random(7);
    int rand_y = random(10);
#endif

    const unsigned char* enemys[] = { waniF, pooka, pookaL, wani, waniL, pooka, pookaL, wani, waniL};
#ifdef WATCHY_SIM
    int rand_enemy = rand() % 9;
#else
    int rand_enemy = random(9);
#endif
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

    display.drawBitmap(INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, enemys[rand_enemy], INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, GxEPD_WHITE);
    if (rand_enemy == 0) {
        display.drawBitmap(INDEX_SIZE * draw_index_X, INDEX_SIZE * draw_index_Y, waniFB, INDEX_SIZE * enemy_size_x, INDEX_SIZE * enemy_size_y, GxEPD_BLACK);
    }


}

const int rock_indexs[8][2] = {
    {3,7},
    {17,11},
    {34,7},
    {43,10},
    {23,34},
    {4,41},
    {26,41},
    {39,40}
};

void WatchyDigdug::drawRock()
{
    int choiced[3] = { -1, -1, -1 };
    int randMax = 8;
    for (int cnt = 0; cnt < 3; ) {
#ifdef WATCHY_SIM
        int choice = rand()% randMax;
#else
        int choice = random(randMax);
#endif
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
    display.drawBitmap(INDEX_SIZE * rock_indexs[choiced[0]][0], INDEX_SIZE * rock_indexs[choiced[0]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
    display.drawBitmap(INDEX_SIZE * rock_indexs[choiced[1]][0], INDEX_SIZE * rock_indexs[choiced[1]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);
    display.drawBitmap(INDEX_SIZE * rock_indexs[choiced[2]][0], INDEX_SIZE * rock_indexs[choiced[2]][1], rock, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK);

}

void WatchyDigdug::drawBattery(){
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
      display.drawBitmap(INDEX_SIZE * 46 - INDEX_SIZE * 4 *batterySegments, INDEX_SIZE * 46, player_rest, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); //player
    }
}

void WatchyDigdug::drawFlower()
{
  if (currentTime.Minute == 0){
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
  if (WIFI_CONFIGURED) {
    display.drawBitmap(INDEX_SIZE * 46, INDEX_SIZE * 2, flower, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); //wifi flower
  }
  if (NTP_SYNC) {
    display.drawBitmap(INDEX_SIZE * 42, INDEX_SIZE * 2, flower, INDEX_SIZE * 4, INDEX_SIZE * 4, GxEPD_BLACK); //ntp flower
  }
#ifndef WATCHY_SIM
  WiFi.mode(WIFI_OFF);
  btStop();
#endif
}

void WatchyDigdug::drawDate() {
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);

    String score = "1P";
    if (currentTime.Month < 10) {
        score += " ";
    }
    score += currentTime.Month;
    if (currentTime.Day < 10) {
        score += "0";
    }
    score += currentTime.Day;
    display.setCursor(INDEX_SIZE * 0, INDEX_SIZE * 4);
    display.print(score);

    String hiscore = " HIGH";
    hiscore += tmYearToCalendar(currentTime.Year);
    display.print(hiscore);
}
