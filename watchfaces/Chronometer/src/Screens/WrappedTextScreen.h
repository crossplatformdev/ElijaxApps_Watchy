#pragma once

#include "Screen.h"

class WrappedTextScreen : public Screen {
 private:
  const char *text;
  const char *altText;

 public:
  WrappedTextScreen(const char *t, const char *at, uint16_t bg = GxEPD_WHITE) : Screen(bg), text(t), altText(at){};
  virtual void show() override;
  virtual void menu() {  // swap the text
    const char *t = text;
    text = altText;
    altText = t;
    Watchy::showWatchFace(true, this);
  };
  void setText(const char *t) { text = t; };
};