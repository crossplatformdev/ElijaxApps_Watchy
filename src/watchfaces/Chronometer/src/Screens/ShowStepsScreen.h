#pragma once

#include "Screen.h"

class ShowStepsScreen : public Screen {
 public:
  ShowStepsScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};
