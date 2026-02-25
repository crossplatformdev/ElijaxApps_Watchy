#pragma once

#include <GxEPD2_BW.h>
#include <GxEPD2.h>

// Wrapper around GxEPD2_BW that can invert black/white at draw time.
//
// This is used to make third-party watchfaces (which often hardcode GxEPD_BLACK/
// GxEPD_WHITE) respect the global theme polarity without modifying each face.

template <typename GxEPD2_Type, const uint16_t page_height>
class ThemeableGxEPD2_BW : public GxEPD2_BW<GxEPD2_Type, page_height> {
 public:
  using Base = GxEPD2_BW<GxEPD2_Type, page_height>;

  explicit ThemeableGxEPD2_BW(GxEPD2_Type epd2_instance) : Base(epd2_instance) {}

  void setInverted(bool inverted) { inverted_ = inverted; }
  bool isInverted() const { return inverted_; }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawPixel(x, y, color);
  }

  void fillScreen(uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::fillScreen(color);
  }

  void setTextColor(uint16_t c) {
    if (inverted_) {
      c = invertColor(c);
    }
    Base::setTextColor(c);
  }

  void setTextColor(uint16_t c, uint16_t bg) {
    if (inverted_) {
      c = invertColor(c);
      bg = invertColor(bg);
    }
    Base::setTextColor(c, bg);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawLine(x0, y0, x1, y1, color);
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawFastHLine(x, y, w, color);
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawFastVLine(x, y, h, color);
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawRect(x, y, w, h, color);
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::fillRect(x, y, w, h, color);
  }

  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawCircle(x0, y0, r, color);
  }

  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::fillCircle(x0, y0, r, color);
  }

  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawRoundRect(x, y, w, h, r, color);
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::fillRoundRect(x, y, w, h, r, color);
  }

  void drawTriangle(int16_t x0,
                    int16_t y0,
                    int16_t x1,
                    int16_t y1,
                    int16_t x2,
                    int16_t y2,
                    uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawTriangle(x0, y0, x1, y1, x2, y2, color);
  }

  void fillTriangle(int16_t x0,
                    int16_t y0,
                    int16_t x1,
                    int16_t y1,
                    int16_t x2,
                    int16_t y2,
                    uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::fillTriangle(x0, y0, x1, y1, x2, y2, color);
  }

  void drawBitmap(int16_t x,
                  int16_t y,
                  const uint8_t bitmap[],
                  int16_t w,
                  int16_t h,
                  uint16_t color) {
    if (inverted_) {
      color = invertColor(color);
    }
    Base::drawBitmap(x, y, bitmap, w, h, color);
  }

  void drawBitmap(int16_t x,
                  int16_t y,
                  const uint8_t bitmap[],
                  int16_t w,
                  int16_t h,
                  uint16_t color,
                  uint16_t bg) {
    if (inverted_) {
      color = invertColor(color);
      bg = invertColor(bg);
    }
    Base::drawBitmap(x, y, bitmap, w, h, color, bg);
  }

 private:
  static uint16_t invertColor(uint16_t color) {
    // For b/w GxEPD2, any non-black value is treated as white. We only need to
    // swap black and white for theme inversion.
    return (color == GxEPD_BLACK) ? GxEPD_WHITE : GxEPD_BLACK;
  }

  bool inverted_ = false;
};
