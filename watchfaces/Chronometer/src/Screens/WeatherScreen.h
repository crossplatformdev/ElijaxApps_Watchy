#pragma once

#include "Screen.h"

class WeatherScreen : public Screen {
 public:
  WeatherScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};
