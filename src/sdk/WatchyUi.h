#ifndef WATCHY_UI_H
#define WATCHY_UI_H

#include <Watchy.h>

namespace WatchyUi {

enum class Event : uint8_t {
  NONE,
  SELECT,
  BACK,
  UP,
  DOWN
};

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
  static void load();
  static bool setDark(bool dark);
#ifdef WATCHY_DETERMINISTIC_GALLERY
  static void useGalleryTheme(bool dark);
#endif
};

class Screen {
public:
  static void beginCanvas();
  static void begin(const char *title);
  static void present(int nextGuiState = APP_STATE);
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

class Input {
public:
  static void begin();
  static Event poll();
  static Event wait(uint32_t timeoutMs = UINT32_MAX);
  static bool pressed(Event event);
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
};

class ListView {
public:
  static void draw(const ListModel &model);
  static void draw(const char *title, const char *const labels[],
                   uint8_t itemCount, uint8_t selectedIndex,
                   const char *footer = nullptr, int16_t activeIndex = -1,
                   uint8_t visibleRows = Theme::listVisibleRows);
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