#pragma once

#include <stdint.h>

struct UIMenuItemSpec;
class Watchy;

namespace WatchfaceRegistry {

struct Entry {
  const char *name;
  void (*draw)(::Watchy &watchy);
};

extern const uint8_t kWatchfaceCount;
extern const UIMenuItemSpec kWatchfaceMenuItems[];
extern const Entry kWatchfaces[];

} // namespace WatchfaceRegistry
