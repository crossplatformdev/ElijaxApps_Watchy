#ifndef UI_SDK_H
#define UI_SDK_H

#include "../watchy/Watchy.h"

// Alias to simplify using the Watchy display type with GxEPD2
using WatchyGxDisplay = ThemeableGxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT>;

// Theme polarity for watchfaces: describes how the asset was authored. Global theme
// (dark/light toggle) can still invert this at runtime; authored polarity is just the
// “native” look of the bitmap/text layout.
enum class WatchfacePolarity : uint8_t {
  WhiteOnBlack,  // White ink on black paper (e.g. 7_SEG, DOS, StarryHorizon)
  BlackOnWhite   // Black ink on white paper (e.g. Pokemon, Tetris, MacPaint, Mario)
};

// Basic UI element types supported by the SDK
enum class UIElementKind : uint8_t {
  Text,
  Button,
  Menu,
  Image,
  // Placeholder for future extension (e.g. Video/Animation)
  Media
};

struct Palette {
  uint16_t fg;
  uint16_t bg;
  uint16_t accentFg;
  uint16_t accentBg;
};

struct UIControlsRowLayout { 
  const char *label; 
  void (Watchy::*callback)();
};

struct UITextSpec {
  int16_t x;
  int16_t y;
  int16_t w;              // optional explicit width (0 = auto from text)
  int16_t h;              // optional explicit height (0 = auto from font)
  const GFXfont *font;
  bool fillBackground;    // if true, fill background before drawing text
  bool invert;            // invert text color vs theme
  String text;
  // NOTE: textColor and backgroundColor are auto-applied by SDK based on current polarity
};

struct UIImageSpec {
  const uint8_t *bitmap;
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  bool fromProgmem;
  bool fillBackground;
  // Polarity the bitmap was authored for; used to determine inversion vs theme
  WatchfacePolarity polarityAuthored = WatchfacePolarity::WhiteOnBlack;
  // NOTE: backgroundColor is auto-applied by SDK based on current polarity
};

struct UIMenuItemSpec {
  String label; // pointer to constant C string
};

struct UIMenuSpec {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  int16_t itemHeight;
  const GFXfont *font;
  const UIMenuItemSpec *items;
  uint8_t itemCount;
  int8_t selectedIndex; // absolute index in items, -1 = nothing selected
  uint8_t startIndex;   // first item index to render (for scrolling menus)
  uint8_t visibleCount; // max rows to render (0 = render all)
  // NOTE: colors are auto-applied by SDK based on current polarity
};

struct UIButtonSpec {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  const GFXfont *font;
  String label;
  bool selected;
  // NOTE: colors are auto-applied by SDK based on current polarity
};

struct UICheckboxSpec {
  int16_t x;
  int16_t y;
  const GFXfont *font;
  bool fillBackground;
  String label;
  bool checked; // true -> "[x]", false -> "[ ]"
  // NOTE: colors are auto-applied by SDK based on current polarity
};

// Simple app definition made of fixed-size arrays of specs.
// This keeps the SDK lightweight and avoids dynamic allocation.
struct UICallbackSpec {
  void (*draw)(WatchyGxDisplay &display, void *userData);
  void *userData;
};

// Scrollable multi-line text area with optional background fill and
// scroll indicators. The text is expected to contain '\n'-separated
// lines. The SDK will render at most maxLines starting at firstLine
// and draw ▲/▼ arrows on the right when there is more content above
// or below the visible window.
struct UIScrollableTextSpec {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  const GFXfont *font;
  bool fillBackground;
  String text;
  // Optional: reference external text to avoid copying large documents.
  // If set, the renderer uses *textRef instead of `text`.
  const String *textRef;
  uint16_t firstLine;  // index of the first line to render
  uint8_t maxLines;    // maximum number of lines to render
  int16_t lineHeight;  // vertical distance in pixels between lines
  bool centered;       // if true, horizontally center each line within w
  // Optional: wrap lines that would exceed the available width.
  // When enabled, long lines are split across multiple visual lines.
  bool wrapLongLines;
  // If wrapLongLines is enabled, continuation segments (2nd+ visual line coming
  // from the same original line) can be rendered right-aligned.
  bool wrapContinuationAlignRight;
  // NOTE: colors are auto-applied by SDK based on current polarity
};

// Standard app chrome (shared look) that can be applied by UiSDK::renderApp.
// This is meant for apps/OS apps (not watchfaces) so they all render with the
// same base appearance and button-hints row.
struct UIAppChromeSpec {
  // If true, renderApp performs the full-screen setup:
  //   - setFullWindow()
  //   - clearScreen(themeBg)
  //   - setTextColor(themeFg)
  bool beginFullScreen = false;

  // If true, renderApp draws the standard controls row after the content.
  bool drawControlsRow = false;

  const char *backLabel = nullptr;
  const char *upLabel = nullptr;
  const char *acceptLabel = nullptr;
  const char *downLabel = nullptr;

  // Panel border waveform control (rarely needed). Defaults to false.
  bool darkBorder = false;
};

struct UIAppSpec {
  const UITextSpec *texts;
  uint8_t textCount;

  const UIImageSpec *images;
  uint8_t imageCount;

  const UIMenuSpec *menus;
  uint8_t menuCount;

  const UIButtonSpec *buttons;
  uint8_t buttonCount;

  const UICheckboxSpec *checkboxes;
  uint8_t checkboxCount;

  const UIScrollableTextSpec *scrollTexts;
  uint8_t scrollTextCount;

  const UICallbackSpec *callbacks;
  uint8_t callbackCount;

  UIControlsRowLayout controls[4];
  uint8_t controlCount;
  // Optional shared chrome that renderApp can apply.
  UIAppChromeSpec chrome{};
};

namespace UiSDK {

// ---------------------------------------------------------------------------
// Font helpers and registry
// ---------------------------------------------------------------------------
// These helpers expose the common bundled fonts and let apps register their
// own GFXfont pointers for reuse without sprinkling includes everywhere.
// Slot 0 is treated as the default when a spec omits a font.
const GFXfont *defaultFont();       // FreeMonoBold9pt7b
const GFXfont *tinyFont5x7();       // bitmap tiny font wrapped for UI specs
const GFXfont *tinyFont6x8();       // proportional tiny font
const GFXfont *tinyMono6x8();       // monospace tiny font

// Register and fetch fonts by small integer slot (0-based). Returns false on
// invalid slot. Stored pointers are used as-is, so ensure they live in memory
// for the lifetime of the app (typically PROGMEM GFXfont structs).
bool registerFont(uint8_t slot, const GFXfont *font);
const GFXfont *getFont(uint8_t slot);

// Theme context management
// setPolarity stores how the current watchface was authored (used for bitmap inversion).
// getThemePolarity reflects the live theme (derived from gDarkMode); getPolarity returns
// the authored polarity last set by the watchface.
// Each watchface is authored with a default polarity (black-on-white or
// white-on-black). We set that context before rendering so the SDK knows how
// to invert bitmaps relative to the current global theme.
void setPolarity(WatchfacePolarity polarity);
WatchfacePolarity getPolarity();
WatchfacePolarity getThemePolarity();
WatchfacePolarity polarityForFaceId(uint8_t id);
WatchfacePolarity liveThemePolarity();

// Some watchfaces are already authored to render using UiSDK theme colors
// (e.g. calling UiSDK::getWatchfaceBg(BASE_POLARITY)). Those should NOT be
// display-inverted again at the OS layer.
bool watchfaceUsesThemeColors(uint8_t id);

// Theme helper functions
// Colors are derived from the provided polarity (typically BASE_POLARITY for live theme).
uint16_t getWatchfaceBg(WatchfacePolarity polarity);
uint16_t getWatchfaceFg(WatchfacePolarity polarity);
// Returns whether a bitmap authored for `polarity` should be inverted for the live theme.
bool shouldInvertBitmap(const Palette &palette); 

// Render primitives
void renderText(WatchyGxDisplay &display, const UITextSpec &spec);
void renderImage(WatchyGxDisplay &display, const UIImageSpec &spec);
void renderMenu(WatchyGxDisplay &display, const UIMenuSpec &spec);
void renderButton(WatchyGxDisplay &display, const UIButtonSpec &spec);
void renderCheckbox(WatchyGxDisplay &display, const UICheckboxSpec &spec);
void renderScrollableText(WatchyGxDisplay &display, const UIScrollableTextSpec &spec);
void renderCallback(WatchyGxDisplay &display, const UICallbackSpec &spec);

// Counts how many visual lines a scroll-text spec will occupy.
// If wrapLongLines is enabled, this includes wrapped continuation lines.
uint16_t countScrollableTextLines(const UIScrollableTextSpec &spec);

// Draw a single line of text at pixel coordinates (x,y). Works with both
// Adafruit GFX fonts and the SDK's built-in tiny bitmap fonts.
// Note: for the tiny bitmap fonts, y is the top pixel coordinate (same as
// renderScrollableText).
void drawTextLine(WatchyGxDisplay &display,
                  int16_t x,
                  int16_t y,
                  const String &text,
                  const GFXfont *font,
                  uint16_t color);


// Input/UI helpers
// Debounced button read (active-low pins by default).
// Shorter default debounce for snappier menu navigation; callers can override per-usage.
bool buttonPressed(uint8_t pin, uint32_t debounceMs = 80, bool activeLow = true);
// Standard control row renderer + input dispatch.
void renderControlsRow(Watchy &watchy, const UIControlsRowLayout layout[4]);

// Inflate and render a complete app on the watchy display.
void renderApp(Watchy &watchy, const UIAppSpec &app);
void renderWatchfaceSpec(Watchy &watchy, const UIAppSpec &app);
void initScreen(WatchyGxDisplay &display);

// ---------------------------------------------------------------------------
// Watchface draw wrappers (SDK function entrypoints for legacy watchfaces)
// ---------------------------------------------------------------------------
inline void init(WatchyGxDisplay &display, uint32_t serial_diag_bitrate = 0, bool initial = true, uint16_t reset_duration = 20, bool pulldown_rst_mode = false) {
  display.init(serial_diag_bitrate, initial, reset_duration, pulldown_rst_mode);
}

inline void setFullWindow(WatchyGxDisplay &display) { display.setFullWindow(); }
inline void setRotation(WatchyGxDisplay &display, uint8_t r) { display.setRotation(r); }
inline void startWrite(WatchyGxDisplay &display) { display.startWrite(); }
inline void endWrite(WatchyGxDisplay &display) { display.endWrite(); }
inline void hibernate(WatchyGxDisplay &display) { display.hibernate(); }
inline int16_t width(WatchyGxDisplay &display) { return display.width(); }
inline int16_t height(WatchyGxDisplay &display) { return display.height(); }
inline void displayUpdate(WatchyGxDisplay &display, bool full = true) { display.display(full); }

inline void drawPixel(WatchyGxDisplay &display, int16_t x, int16_t y, uint16_t color) { display.drawPixel(x, y, color); }
inline void writePixel(WatchyGxDisplay &display, int16_t x, int16_t y, uint16_t color) { display.writePixel(x, y, color); }
inline void fillScreen(WatchyGxDisplay &display, uint16_t color) { display.fillScreen(color); }
inline void fillRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { display.fillRect(x, y, w, h, color); }
inline void fillRoundRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) { display.fillRoundRect(x, y, w, h, r, color); }
inline void fillCircle(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t r, uint16_t color) { display.fillCircle(x, y, r, color); }
inline void fillTriangle(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) { display.fillTriangle(x0, y0, x1, y1, x2, y2, color); }
inline void drawLine(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { display.drawLine(x0, y0, x1, y1, color); }
inline void drawFastHLine(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, uint16_t color) { display.drawFastHLine(x, y, w, color); }
inline void drawFastVLine(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t h, uint16_t color) { display.drawFastVLine(x, y, h, color); }
inline void writeFastVLine(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t h, uint16_t color) { display.writeFastVLine(x, y, h, color); }
inline void drawRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { display.drawRect(x, y, w, h, color); }
inline void drawRoundRect(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) { display.drawRoundRect(x, y, w, h, r, color); }
inline void drawCircle(WatchyGxDisplay &display, int16_t x, int16_t y, int16_t r, uint16_t color) { display.drawCircle(x, y, r, color); }
inline void drawCircleHelper(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) { display.drawCircleHelper(x0, y0, r, cornername, color); }
inline void drawTriangle(WatchyGxDisplay &display, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) { display.drawTriangle(x0, y0, x1, y1, x2, y2, color); }

inline void drawBitmap(WatchyGxDisplay &display, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
  display.drawBitmap(x, y, bitmap, w, h, color);
}

inline void drawBitmap(WatchyGxDisplay &display,
                       int16_t x,
                       int16_t y,
                       const uint8_t *bitmap,
                       int16_t w,
                       int16_t h,
                       uint16_t color,
                       uint16_t bg) {
  display.drawBitmap(x, y, bitmap, w, h, color, bg);
}

inline void setFont(WatchyGxDisplay &display, const GFXfont *font) { display.setFont(font); }
inline void setFont(WatchyGxDisplay &display) { display.setFont(); }
inline void setCursor(WatchyGxDisplay &display, int16_t x, int16_t y) { display.setCursor(x, y); }
inline int16_t getCursorX(WatchyGxDisplay &display) { return display.getCursorX(); }
inline void setTextColor(WatchyGxDisplay &display, uint16_t c) { display.setTextColor(c); }
inline void setTextColor(WatchyGxDisplay &display, uint16_t c, uint16_t bg) { display.setTextColor(c, bg); }
inline void setTextSize(WatchyGxDisplay &display, uint8_t s) { display.setTextSize(s); }
inline void setTextWrap(WatchyGxDisplay &display, bool w) { display.setTextWrap(w); }

template <typename T>
inline size_t print(WatchyGxDisplay &display, const T &value) {
  return display.print(value);
}

template <typename T>
inline size_t println(WatchyGxDisplay &display, const T &value) {
  return display.println(value);
}

template <typename T>
inline void getTextBounds(WatchyGxDisplay &display,
                          const T &text,
                          int16_t x,
                          int16_t y,
                          int16_t *x1,
                          int16_t *y1,
                          uint16_t *w,
                          uint16_t *h) {
  display.getTextBounds(text, x, y, x1, y1, w, h);
}

} // namespace UiSDK


// Determine base polarity from global dark mode setting (evaluated at runtime)
extern bool gDarkMode;  // Global theme toggle from Watchy.cpp
#define BASE_POLARITY (gDarkMode ? WatchfacePolarity::WhiteOnBlack : WatchfacePolarity::BlackOnWhite)

struct UIControls {
  UIControlsRowLayout controls[4];
};

#endif // UI_SDK_H
