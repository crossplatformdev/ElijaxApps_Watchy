#pragma once

#include <Adafruit_GFX.h>

/**
 * @brief draw word wrapped text within a bounding box. Takes GFX, bounding box,
 *        text string, and font. Handles embedded LF, words that won't fit on one
 *        line (hard wraps), text that won't all fit (fits as much as possible),
 *        excess whitespace
 *
 * @param g Adafruit_GFX to draw on
 * @param x upper left of bounding box (left)
 * @param y upper left of bounding box (top)
 * @param w width of bounding box
 * @param h height of bounding box
 * @param t text to draw. will draw as much as fits
 * @param f font to use
 */
void drawWordWrappedText(Adafruit_GFX &g, int16_t x, int16_t y, int16_t w,
                         int16_t h, const char *t, const GFXfont *f);
