#ifndef WATCHY_UI_H
#define WATCHY_UI_H

#include "WatchySdk.h"
#include "Watchy.h"
#include <config.h>
#include "MenuModel.h"

namespace WatchyUi {

enum class Event : uint64_t {
  UP = UP_BTN_MASK,
  DOWN = DOWN_BTN_MASK,
  MENU = MENU_BTN_MASK,
  BACK = BACK_BTN_MASK,
  NONE = 200,
};

enum class WakeupReason : uint8_t {
  NO_DELAY,
  DEEP_SLEEP_DELAY,
  SCHEDULER_DELAY,
  BACK_PRESSED
};

WakeupReason deepSleepDelay(uint32_t durationMs);

enum class MessageKind : uint8_t {
  INFO,
  SUCCESS,
  WARNING,
  ERROR,
  LOADING,
  EMPTY
};

struct Bounds {
  int16_t x;
  int16_t y;
  int16_t width;
  int16_t height;

  Bounds inset(int16_t amount) const;
};

using Gray8 = uint8_t;

constexpr Gray8 GRAY_BLACK = 0;
constexpr Gray8 GRAY_WHITE = 255;

enum class ToneRole : uint8_t {
  Background,
  Surface,
  SurfaceRaised,
  Foreground,
  SecondaryText,
  Muted,
  Disabled,
  Separator,
  Accent,
  Selection
};

class GrayPaint {
public:
  static bool pixelIsWhite(int16_t x, int16_t y, Gray8 gray);
  static void pixel(int16_t x, int16_t y, Gray8 gray);
  static void fillRect(const Bounds &bounds, Gray8 gray);
  static void fillRoundRect(const Bounds &bounds, int16_t radius,
                            Gray8 gray);
  static void line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                   Gray8 gray);
  static void bitmap8(int16_t x, int16_t y, uint16_t width,
                      uint16_t height, const uint8_t *pixels,
                      bool programMemory = false);
  static void gradient(const Bounds &bounds, Gray8 start, Gray8 end,
                       bool horizontal = true);
};

class Canvas {
public:
  static void outline(const Bounds &bounds, uint16_t color);
  static void circle(const Bounds &bounds, uint16_t color,
                     bool filled = false);
  static void centeredText(const Bounds &bounds, const char *text,
                           uint16_t color);
  static void centeredText(const Bounds &bounds, const char *text,
                           uint8_t textSize, uint16_t color);
};

struct Theme {
  static constexpr int16_t screenMargin = 4;
  static constexpr int16_t titleBaseline = 15;
  static constexpr int16_t contentBaseline = 38;
  static constexpr int16_t listFirstBaseline = 47;
  static constexpr int16_t footerBaseline = 190;
  static constexpr int16_t listRowHeight = 22;
  static constexpr uint8_t listVisibleRows = 6;

  static uint16_t background();
  static uint16_t foreground();
  static Gray8 tone(ToneRole role);
  static void load();
  static bool setDark(bool dark);
#ifdef WATCHY_DETERMINISTIC_GALLERY
  static void useGalleryTheme(bool dark);
#endif
};

class Screen {
public:
  static constexpr uint32_t liveViewRefreshIntervalMs = 1000;
  static constexpr uint8_t maximumDirtyRegions = 4;

  static void beginCanvas();
  static void begin(const char *title);
  static void invalidate(const Bounds &bounds);
  static void presentDirty(int nextGuiState = APP_STATE);
  // allowPeriodicFullRefresh permits an occasional full flashing refresh to
  // clear ghosting; the watch face disables this and only ever gets a full
  // refresh for its very first boot paint.
  static void present(int nextGuiState = APP_STATE,
                      bool allowPeriodicFullRefresh = false);
  static void present(const Bounds &dirtyBounds,
                      int nextGuiState = APP_STATE);
};

class Power {
public:
  static void idle(uint32_t durationMs);
  static bool usbPluggedIn();
  static void waitForDisplayReady();
  static bool waitForDisplayReady(uint32_t timeoutMs);
};

class Widget {
public:
  static void bodyText(int16_t x = Theme::screenMargin,
                       int16_t y = Theme::contentBaseline);
  static void separator(int16_t y = 23);
  static void footer(const char *commands);
  static void paragraph(const char *text, int16_t x, int16_t y,
                        uint8_t columns, uint8_t maximumLines,
                        uint8_t lineHeight = 9);
  static void progress(float value, int16_t y);
  static void checkbox(const char *label, bool checked, int16_t y,
                       bool focused = false, bool enabled = true);
  static void radio(const char *label, bool selected, int16_t y,
                    bool focused = false, bool enabled = true);
  static void toggle(bool enabled, int16_t x, int16_t y,
                     bool focused = false);
};

struct ValueModel {
  const char *title;
  const char *value;
  const char *status;
  const char *detail;
  const char *footer;
  float progress;

  ValueModel(const char *modelTitle, const char *modelValue,
             const char *modelStatus, const char *modelDetail,
             const char *modelFooter, float modelProgress = -1.0f)
      : title(modelTitle), value(modelValue), status(modelStatus),
        detail(modelDetail), footer(modelFooter), progress(modelProgress) {}
};

class ValueView {
public:
  static void draw(const ValueModel &model);
};

class Feedback {
public:
  static void showMessage(const char *title, const char *message,
                          MessageKind kind = MessageKind::INFO,
                          const char *footer = nullptr);
  static bool confirm(const char *title, const char *message);
  static void toast(const char *message);
};

struct ScrollableTextModel {
  const char *title;
  const char *text;
  const char *footer;
  uint16_t firstLine;
  uint8_t columns;
  uint8_t visibleLines;
  uint8_t lineHeight;
};

class ScrollableTextView {
public:
  static uint16_t lineCount(const char *text, uint8_t columns = 31);
  static uint16_t maximumFirstLine(uint16_t totalLines,
                                   uint8_t visibleLines = 16);
  static uint16_t previous(uint16_t firstLine,
                           uint8_t visibleLines = 16);
  static uint16_t next(uint16_t firstLine, uint16_t totalLines,
                       uint8_t visibleLines = 16);
  static void draw(const ScrollableTextModel &model);
  static void show(const char *title, const char *text,
                   const char *footer = "UP/DOWN SCROLL     BACK EXIT",
                   uint8_t columns = 31, uint8_t visibleLines = 16,
                   uint8_t lineHeight = 9);
};

class Input {
public:
  using WakeHandler = void (*)();

  static void begin();
  static void setAuxiliaryWakeSource(uint8_t pin, uint8_t activeLevel,
                                     WakeHandler handler);
  static Event poll();
  static Event wait(uint32_t timeoutMs = UINT32_MAX);
  static Event waitScheduled(uint32_t timeoutMs = UINT32_MAX);
  static Event waitNotified(uint32_t timeoutMs = UINT32_MAX);
  static void waitForRelease(Event event);
};

using LabelProvider = const char *(*)(uint8_t index, const void *context);

struct ListModel {
  const char *title;
  LabelProvider labelAt;
  LabelProvider detailAt;
  const void *context;
  const char *footer;
  uint8_t itemCount;
  uint8_t selectedIndex;
  uint8_t visibleRows;
  int16_t activeIndex;
  bool showDisclosure;
  bool compactText;
};

class ListView {
public:
  static void draw(const ListModel &model);
  static void draw(const char *title, const char *const labels[],
                   uint8_t itemCount, uint8_t selectedIndex,
                   const char *footer = nullptr, int16_t activeIndex = -1,
                   uint8_t visibleRows = Theme::listVisibleRows);
  static void presentSelectionChange(const ListModel &model,
                                     uint8_t previousSelectedIndex,
                                     int nextGuiState = APP_STATE);
  static void presentSelectionChange(
      const char *title, const char *const labels[], uint8_t itemCount,
      uint8_t selectedIndex, uint8_t previousSelectedIndex,
      const char *footer = nullptr, int16_t activeIndex = -1,
      uint8_t visibleRows = Theme::listVisibleRows,
      int nextGuiState = APP_STATE);
  static uint8_t previous(uint8_t selectedIndex, uint8_t itemCount);
  static uint8_t next(uint8_t selectedIndex, uint8_t itemCount);
};

class Selector {
public:
  static int32_t step(int32_t value, int32_t amount, int32_t minimum,
                      int32_t maximum, bool wrap = false);
  static void formatTime(char output[6], uint8_t hour, uint8_t minute);
};

} // namespace WatchyUi

#endif