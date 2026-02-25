#pragma once

#include "Adafruit_GFX.h"

// represents a simple rle encoded image. w and h are the width and height of
// the image (max 255 x 255) and bitmap represents runs of bits. The first
// uint8_t is a count of transparent bits, then a count of non-transparent
// bits, then transparent, and so on. The bits are recorded as one long stream
// of bits concatenated together in row order.

// a 10 x 10 image of a square:
//
// . . . . . . . . . .
// . . . . . . . . . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . X X X X X X . .
// . . . . . . . . . .
// . . . . . . . . . .
//
// turns into
// 
// ......................XXXXXX....XXXXXX....XXXXXX....XXXXXX....XXXXXX....XXXXXX....XXXXXX....XXXXXX......................
//
// and would be represented as { 10, 10, &{22,6,4,6,4,6,4,6,4,6,4,6,4,6,4,6,22} }

// this is only useful if most of the runs are longer than 8 bits - so simple
// solid figures compress best

// there's a PNG to RLE converter in go at https://github.com/charles-haynes/png2rle

typedef struct {
  const uint8_t w, h;
  const uint8_t *bitmap;
} rle;

void drawRLEBitmap(Adafruit_GFX &gfx, int16_t x, int16_t y, const rle *r,
                   uint16_t color);
