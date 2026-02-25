#pragma once

#include "Screen.h"

class SetTimeScreen : public Screen {
 private:
  static uint8_t setIndex;

 public:
  int16_t minute;
  int16_t hour;
  int16_t day;
  int16_t month;
  int16_t year;

 public:
  SetTimeScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
  void up() override;
  void down() override;
  void back() override;
  void menu() override;
};