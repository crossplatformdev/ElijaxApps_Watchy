#pragma once

#include "../../../watchy/Watchy.h"

// Abstract base class for screens. Each screen must provide a method for
// showing its face, and methods for handling up, down, back, and menu button
// presses. Default handlers set watchy screen to that screen.
class Screen {
 public:
  Screen* parent;
  uint16_t bgColor;
  Screen(uint16_t bg = GxEPD_WHITE) : parent(nullptr), bgColor(bg){};
  virtual void show() = 0;  // display this screen
  virtual void up() {}
  virtual void down() {}
  virtual void back() { Watchy::setScreen(parent); }
  virtual void menu() {}
};