#pragma once

#include "Screen.h"

class SyncTimeScreen : public Screen {
 public:
  SyncTimeScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
  void back() override;
};