#ifndef WATCHY_UTILITY_SUPPORT_H
#define WATCHY_UTILITY_SUPPORT_H

#include <WatchySdk.h>

namespace WatchyUtilityTools {

using Generator = void (*)(char *output, size_t outputSize);
using GeneratorRenderer = void (*)(const char *value);
using ValueRenderer = void (*)(int value);

void runGenerator(Generator generator, GeneratorRenderer renderer);
void runAdjustableValue(int initialValue, int step, int minimum, int maximum,
                        ValueRenderer renderer);
void useSmallText(int16_t x = 4, int16_t y = 38);
void drawCoinFace(const char *result);
void drawDieFace(uint8_t sides, uint8_t value);
void drawConverterArrow();

} // namespace WatchyUtilityTools

#endif