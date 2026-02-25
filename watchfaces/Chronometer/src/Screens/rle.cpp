#include "rle.h"

#include "Adafruit_GFX.h"
#include "GxEPD2.h"

// this implements a simple RLE encoding. It expects an Adafruit GFX drawable,
// the position to draw the image at, a pointer to a rle struct, and the color
// of the image.

// does no sanity checking, length of bitmap, size of gfx
void drawRLEBitmap(Adafruit_GFX &gfx, int16_t x, int16_t y, const rle *r,
                   uint16_t color) {
  bool transparent = true;
  const uint8_t *run = &(r->bitmap[0]);
  uint8_t runCount = *run;
  run++;
  for (uint8_t y0 = 0; y0 < r->h; y0++) {
    for (uint8_t x0 = 0; x0 < r->w; x0++) {
      while (runCount == 0) {
        runCount = *run;
        run++;
        transparent = !transparent;
      }
      if (!transparent) {
        gfx.drawPixel(x + x0, y + y0, color);
      }
      runCount--;
    }
  }
}