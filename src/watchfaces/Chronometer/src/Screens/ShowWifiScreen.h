#pragma once

#include "Screen.h"

class ShowWifiScreen : public Screen {
 public:
  ShowWifiScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};
