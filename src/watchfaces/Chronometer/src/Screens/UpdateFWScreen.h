#pragma once

#include "Screen.h"

class UpdateFWScreen : public Screen {
 public:
  UpdateFWScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
  void menu() override;
};