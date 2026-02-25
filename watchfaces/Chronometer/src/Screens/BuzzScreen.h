#pragma once

#include "Screen.h"

class BuzzScreen : public Screen {
 public:
  BuzzScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};
