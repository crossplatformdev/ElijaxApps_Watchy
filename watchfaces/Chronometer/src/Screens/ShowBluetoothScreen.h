#pragma once

#include "Screen.h"

class ShowBluetoothScreen : public Screen {
 public:
  ShowBluetoothScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};