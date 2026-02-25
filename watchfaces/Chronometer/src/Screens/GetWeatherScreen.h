#pragma once

#include "Screen.h"

class GetWeatherScreen : public Screen {
 public:
  GetWeatherScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
  void back() override;
};