#include "MarqueeWatch.h"

#include "../../../sdk/UiSDK.h"

#include "bitmaps.h"
#include "constants.h"
#include "fonts.h"

#define BATTERY_OFFSET 0.00  // This offset is different for every Watchy

#define MAXIMUM_NUMBER_OF_BIRDS 6
#define MAXIMUM_NUMBER_OF_LEAFS_PER_VINE 34

MarqueeWatch::MarqueeWatch(const watchySettings& settings)
  : Watchy(settings) {}

// Overrides
void MarqueeWatch::drawWatchFace() {
  UiSDK::initScreen(display);
  const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const uint16_t bgColor = UiSDK::getWatchfaceBg(BASE_POLARITY);

  drawBackground(fgColor, bgColor);
  drawBatteryBirds(bgColor);
  drawDate(fgColor);
  drawTime(fgColor);
  drawVines(fgColor, bgColor);
}

// Methods
void MarqueeWatch::drawBackground(uint16_t fgColor, uint16_t bgColor) {
  UiSDK::fillScreen(display, bgColor);
  UiSDK::drawBitmap(display, 0, 0, BITMAP_BACKGROUND, DISPLAY_WIDTH, DISPLAY_HEIGHT, fgColor);
}

void MarqueeWatch::drawBatteryBirds(uint16_t bgColor) {
  // See how many birds we have to show
  int numberOfBirds = (int)ceil(getBatteryPercentage() / (100 / MAXIMUM_NUMBER_OF_BIRDS));

  // Clip them birds just to be safe
  numberOfBirds = min(6, max(1, numberOfBirds));

  // Check if we need to do anything
  if(6 == numberOfBirds) {
    return;
  }

  // Let's hide some birds! (bottom right 144, 57)
  int x = BATTERY_BIRD_POSITIONS_X[numberOfBirds];
  UiSDK::fillRect(display, x, 47, 144 - x, 10, bgColor);
}

void MarqueeWatch::drawDate(uint16_t fgColor) {
  // Draw the short name of the month
  const int* shortNameIndexes = DATE_TO_SHORTNAME_INDEXES[currentTime.Month - 1];
  UiSDK::drawBitmap(display, 34, 106, ISOMETRIC_DOWN_GLYPH_FONT[shortNameIndexes[0]], ISOMETRIC_DOWN_FONT_WIDTH, ISOMETRIC_DOWN_FONT_HEIGHT, fgColor);
  UiSDK::drawBitmap(display, 42, 110, ISOMETRIC_DOWN_GLYPH_FONT[shortNameIndexes[1]], ISOMETRIC_DOWN_FONT_WIDTH, ISOMETRIC_DOWN_FONT_HEIGHT, fgColor);
  UiSDK::drawBitmap(display, 50, 114, ISOMETRIC_DOWN_GLYPH_FONT[shortNameIndexes[2]], ISOMETRIC_DOWN_FONT_WIDTH, ISOMETRIC_DOWN_FONT_HEIGHT, fgColor);

  // Draw the numbers of the date
  int day = currentTime.Day;
  UiSDK::drawBitmap(display, 148, 113, ISOMETRIC_UP_GLYPH_FONT[getTens(day)], ISOMETRIC_UP_FONT_WIDTH, ISOMETRIC_UP_FONT_HEIGHT, fgColor);
  UiSDK::drawBitmap(display, 156, 109, ISOMETRIC_UP_GLYPH_FONT[day % 10], ISOMETRIC_UP_FONT_WIDTH, ISOMETRIC_UP_FONT_HEIGHT, fgColor);
}

void MarqueeWatch::drawTime(uint16_t fgColor) {
  // Set the current hour per digit
  UiSDK::drawBitmap(display, 64, 68, NEON_FONT[getTens(currentTime.Hour)], NEON_FONT_WIDTH, NEON_FONT_HEIGHT, fgColor);
  UiSDK::drawBitmap(display, 80, 68, NEON_FONT[currentTime.Hour % 10], NEON_FONT_WIDTH, NEON_FONT_HEIGHT, fgColor);

  // Set the current minute per digit
  UiSDK::drawBitmap(display, 105, 68, NEON_FONT[getTens(currentTime.Minute)], NEON_FONT_WIDTH, NEON_FONT_HEIGHT, fgColor);
  UiSDK::drawBitmap(display, 122, 68, NEON_FONT[currentTime.Minute % 10], NEON_FONT_WIDTH, NEON_FONT_HEIGHT, fgColor);
}

void MarqueeWatch::drawVines(uint16_t fgColor, uint16_t bgColor) {
  // Step counter managed by core Watchy

  // Get the actual amount of steps that have been taken
  int steps = (int)sensor.getCounter();
  
  // Or: Simulate it for testing purposes
  // int steps = currentTime.Minute * 567;  // Will be between 0 and (60 * 567 =) 34020
  int numberOfLeafs = (int)(steps / 500);

  // Add a nice little early out
  if (0 == numberOfLeafs) {
    // Block out the vines pretty much completely
    UiSDK::fillRect(display, 10, 24, 4, 136, bgColor);
    UiSDK::fillRect(display, 186, 24, 4, 136, bgColor);

    return;
  }

  // Cap it for now at a measily 34000 steps
  numberOfLeafs = min(numberOfLeafs, MAXIMUM_NUMBER_OF_LEAFS_PER_VINE * 2);

  // Check how many leafs we are above 17000 steps
  int numberOfLeafsLeft = numberOfLeafs - MAXIMUM_NUMBER_OF_LEAFS_PER_VINE;

  // Block out the vines until the point of interest
  UiSDK::fillRect(display, 10, 24, 4, 136 - (((min(numberOfLeafs, MAXIMUM_NUMBER_OF_LEAFS_PER_VINE)) - 1) * 4), bgColor);
  UiSDK::fillRect(display, 186, 24, 4, 136 - (((numberOfLeafsLeft > 0 ? numberOfLeafsLeft : numberOfLeafs) - 1) * 4), bgColor);

  // Draw in the leafs where needed
  for (int index = 0; index < min(numberOfLeafs, MAXIMUM_NUMBER_OF_LEAFS_PER_VINE); ++index) {
    int leafIndex = index % 4;

    // Left vine: left, right, right, left
    UiSDK::drawBitmap(display, LEFT_VINE_POSITIONS_X[leafIndex], 161 - (index * 4), (0 == leafIndex || 3 == leafIndex ? BITMAP_LEAF_LEFT : BITMAP_LEAF_RIGHT), 3, 3, fgColor);

    // Check if we're over 17000 steps, otherwise sync them
    // Right vine: right, left, left, right
    if (numberOfLeafsLeft <= 0 || index <= numberOfLeafsLeft) {
      UiSDK::drawBitmap(display, RIGHT_VINE_POSITIONS_X[leafIndex], 161 - (index * 4), (0 == leafIndex || 3 == leafIndex ? BITMAP_LEAF_RIGHT : BITMAP_LEAF_LEFT), 3, 3, fgColor);
    }
  }
}

int MarqueeWatch::getBatteryPercentage() {
  // Thanks to https://github.com/peerdavid/wos for most of this snippet of code!
  float voltage = getBatteryVoltage() + BATTERY_OFFSET;
  uint8_t percentage = 2808.3808 * pow(voltage, 4)
                       - 43560.9157 * pow(voltage, 3)
                       + 252848.5888 * pow(voltage, 2)
                       - 650767.4615 * voltage
                       + 626532.5703;
  return (int)min((uint8_t)100, max((uint8_t)0, percentage));
}

int MarqueeWatch::getTens(int value) {
  return (int)((value / 10) % 10);
}

void showWatchFace_Marquee(Watchy &watchy) {
  MarqueeWatch face(watchy.settings);
  face.currentTime = watchy.currentTime;
  face.drawWatchFace();
}
