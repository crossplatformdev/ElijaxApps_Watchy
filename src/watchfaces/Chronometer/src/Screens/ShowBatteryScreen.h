#pragma once

#include "Screen.h"

class ShowBatteryScreen : public Screen {
 public:
  ShowBatteryScreen(uint16_t bg = GxEPD_WHITE);
  void show() override;
};
