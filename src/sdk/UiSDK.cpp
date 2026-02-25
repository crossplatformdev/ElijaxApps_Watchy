#include "UiSDK.h"
#include "Fonts.h"

#include "../watchy/Watchy.h"

namespace {

constexpr bool isDark(WatchfacePolarity polarity) {
  return polarity == WatchfacePolarity::WhiteOnBlack;
}

Palette paletteFor(WatchfacePolarity polarity) {
  bool dark = isDark(polarity);
  Palette p{};
  p.bg = dark ? GxEPD_BLACK : GxEPD_WHITE;
  p.fg = dark ? GxEPD_WHITE : GxEPD_BLACK;
  // Accent colours are inverted for selected states (menus, buttons).
  p.accentBg = p.fg;
  p.accentFg = p.bg;
  return p;
}

inline uint8_t readByte(const uint8_t *ptr, bool fromProgmem) {
  return fromProgmem ? pgm_read_byte(ptr) : *ptr;
}

void drawBitmapUnaligned(WatchyGxDisplay &display, const UIImageSpec &spec) {
  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 
  uint16_t onColor =  palette.fg; 
  uint16_t offColor = palette.bg; 
  
  if(spec.fillBackground){
    display.fillRect(spec.x, spec.y, spec.w, spec.h, offColor); 
  }

  const uint16_t bytesPerRow = (spec.w + 7) / 8;
  for (int16_t y = 0; y < spec.h; ++y) {
    for (int16_t x = 0; x < spec.w; ++x) {
      const uint16_t byteIndex = y * bytesPerRow + (x >> 3);
      const uint8_t bits = readByte(spec.bitmap + byteIndex, spec.fromProgmem);
      const bool bitSet = bits & (0x80 >> (x & 7));
      // Transparent bitmap semantics (match AdafruitGFX drawBitmap):
      // - draw set bits with onColor
      // - leave unset bits unchanged (unless fillBackground prefilled)
      if (bitSet) {
        display.drawPixel(spec.x + x, spec.y + y, onColor);
      }
    }
  }
}

struct DebounceSlot {
  uint8_t pin;
  bool used;
  bool latched;
  uint32_t lastEventMs;
};

static DebounceSlot gDebounceSlots[16] = {
    {0, false, false, 0}, {0, false, false, 0}, {0, false, false, 0},
    {0, false, false, 0}, {0, false, false, 0}, {0, false, false, 0},
    {0, false, false, 0}, {0, false, false, 0}, {0, false, false, 0},
    {0, false, false, 0}, {0, false, false, 0}, {0, false, false, 0},
    {0, false, false, 0}, {0, false, false, 0}, {0, false, false, 0},
    {0, false, false, 0}};

static DebounceSlot *acquireDebounceSlot(uint8_t pin) {
  DebounceSlot *freeSlot = nullptr;
  for (auto &slot : gDebounceSlots) {
    if (slot.used && slot.pin == pin) {
      return &slot;
    }
    if (!slot.used && freeSlot == nullptr) {
      freeSlot = &slot;
    }
  }
  if (freeSlot != nullptr) {
    freeSlot->used = true;
    freeSlot->pin = pin;
    freeSlot->latched = false;
    freeSlot->lastEventMs = 0;
    return freeSlot;
  }
  return &gDebounceSlots[0];
}

static void drawTinyChar(WatchyGxDisplay &display, int16_t x, int16_t y, char c, uint16_t color) {
  if (c < 0x20 || c > 0x7E) {
    return;
  }
  const uint8_t *glyph = sTinyFont6x8[c - 0x20];
  for (uint8_t col = 0; col < 6; ++col) {
    uint8_t bits = pgm_read_byte(&glyph[col]);
    for (uint8_t row = 0; row < 8; ++row) {
      if (bits & 0x01) {
        display.drawPixel(x + col, y + row, color);
      }
      bits >>= 1;
    }
  }
}

static void drawTinyText(WatchyGxDisplay &display, int16_t x, int16_t y, const String &text, uint16_t color) {
  int16_t cursorX = x;
  for (uint16_t i = 0; i < text.length(); ++i) {
    char c = text[i];
    if (c == '\n') {
      break; // caller moves Y
    }
    drawTinyChar(display, cursorX, y, c, color);
    cursorX += 5; // 4px glyph + 1px spacing
    if (cursorX > WatchyDisplay::WIDTH) {
      break;
    }
  }
}

// Font registry: slot 0 is default; slots 1..n are optional app-registered fonts.
// Bump slot count so projects can register many of the bundled Adafruit GFX fonts
// defined in Fonts.h without running out of space.
constexpr uint8_t kFontSlotCount = 64;
static const GFXfont *gFontSlots[kFontSlotCount] = {};

// Sentinel pointers that let the renderer recognize the built-in tiny bitmaps.
static const GFXfont *const gTinyFont5x7 = reinterpret_cast<const GFXfont *>(sTinyFont5x7);
static const GFXfont *const gTinyFont6x8 = reinterpret_cast<const GFXfont *>(sTinyFont6x8);
static const GFXfont *const gTinyMono6x8 = reinterpret_cast<const GFXfont *>(sTinyMonospaceFont6x8);

static void initFontRegistryOnce() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  gFontSlots[0] = &FreeMonoBold9pt7b;
  gFontSlots[1] = gTinyFont6x8;
  gFontSlots[2] = gTinyMono6x8;
  gFontSlots[3] = gTinyFont5x7;
  initialized = true;
}

} // namespace

// Authored polarity context (what the current watchface/assets were authored for).
// This is independent from the live theme (gDarkMode).
static WatchfacePolarity gAuthoredPolarity = WatchfacePolarity::WhiteOnBlack;

// Theme context management
void UiSDK::setPolarity(WatchfacePolarity polarity) {
  gAuthoredPolarity = polarity;
}

WatchfacePolarity UiSDK::getPolarity() {
  return gAuthoredPolarity;
}

WatchfacePolarity UiSDK::getThemePolarity() {
  return BASE_POLARITY;
}

// Each watchface is authored with a default polarity (black-on-white or
// white-on-black). We set that context before rendering so the SDK knows how
// to invert bitmaps relative to the current global theme.
WatchfacePolarity UiSDK::polarityForFaceId(uint8_t id) {
  // IMPORTANT:
  // - This is the authored polarity of each watchface ("native" look).
  // - At runtime, we may invert output when BASE_POLARITY differs.
  //
  // Default: most Watchy faces are authored black ink on white paper.
  //
  // This table was derived by scanning each watchface's source for clear
  // background and draw color usage (fillScreen/drawBitmap/setTextColor).
  // See tools/watchface_polarities_report.md and tools/derive_watchface_polarities.py.
  switch (id) {
    case 2:  // Analog
    case 3:  // Bad_For_Eye
    case 7:  // beastie
    case 8:  // Big_Time
    case 9:  // Binary
    case 16: // Calendar_WatchFace
    case 17: // Captn_Wednesday
    case 18: // Castle_of_Watchy
    case 23: // dkTime
    case 25: // DOS
    case 31: // Keen
    case 32: // Kitty
    case 39: // Maze
    case 42: // Multi_face_Watchy
    case 47: // pxl999
    case 51: // Re-Dub
    case 56: // Skully
    case 57: // Skykid_Watch
    case 64: // StarryHorizon
    case 65: // Stationary_Text
    case 67: // SW_Watchy
    case 68: // Sundial
    case 73: // Watchy_Akira
    case 74: // Watchy_PowerShell
    case 76: // X_marks_the_spot
      return WatchfacePolarity::WhiteOnBlack;

    default:
      return WatchfacePolarity::BlackOnWhite;
  }
}

WatchfacePolarity UiSDK::liveThemePolarity() {
  return BASE_POLARITY;
}

bool UiSDK::watchfaceUsesThemeColors(uint8_t id) {
  // Faces that already use UiSDK theme palette internally should not be
  // display-inverted again.
  //
  // Keep this list small and explicit; most third-party faces hardcode
  // GxEPD_BLACK/GxEPD_WHITE and rely on OS-level inversion.
  switch (id) {
    case 0:  // 7_SEG (rewritten to UiSDK primitives)
    case 1:  // 7_SEG_LIGHT (rewritten to UiSDK primitives)
    case 2:  // Analog (theme-driven)
    case 3:  // Bad_For_Eye (rewritten to UiSDK theme colors)
    case 4:  // Bahn (theme-driven)
    case 6:  // BCD (theme-driven via UiSDK-backed BCD base)
    case 7:  // beastie (theme-driven)
    case 8:  // Big_Time (rewritten to UiSDK theme colors)
    case 9:  // Binary (rewritten to UiSDK theme colors)
    case 10: // BinaryBlocks (theme-driven)
    case 11: // BotWatchy (theme-driven)
    case 12: // Brainwork (theme-driven)
    case 13: // BTTF (theme-driven via gDarkMode -> bcd_dark_mode)
    case 14: // Calculator_Watchy (theme-driven)
    case 15: // Calculateur (theme-driven)
    case 16: // Calendar_WatchFace (theme-driven)
    case 17: // Captn_Wednesday (theme-driven)
    case 18: // Castle_of_Watchy (theme-driven)
    case 19: // Chaos_-_Lorenz_Attractor (theme-driven)
    case 20: // Chronometer (theme-driven)
    case 21: // CityWeather (theme-driven)
    case 22: // Dali (theme-driven)
    case 23: // dkTime (theme-driven)
    case 24: // Digdug (theme-driven)
    case 25: // DOS (theme-driven)
    case 26: // erika_Type (theme-driven)
    case 27: // Exactly-Words (theme-driven)
    case 28: // Hobbit_Time (theme-driven)
    case 29: // Jarvis (theme-driven)
    case 30: // Kave_Watchy (theme-driven)
    case 31: // Keen (theme-driven)
    case 32: // Kitty (theme-driven)
    case 33: // Last_Laugh (theme-driven)
    case 34: // LCARS (theme-driven)
    case 35: // Line (theme-driven)
    case 36: // MacPaint (theme-driven)
    case 38: // Marquee (theme-driven)
    case 39: // Maze (theme-driven)
    case 40: // MetaBall (theme-driven)
    case 41: // Mickey (theme-driven)
    case 42: // Multi_face_Watchy (theme-driven via BASE_POLARITY)
    case 43: // Orbital (theme-driven)
    case 44: // Pip-Boy (theme-driven)
    case 45: // Pokemon (theme-driven)
    case 46: // Poe (theme-driven)
    case 48: // QArtCode (theme-driven)
    case 49: // QLock (theme-driven)
    case 50: // QR_Watchface (theme-driven)
    case 52: // Revolution (theme-driven)
    case 53: // S2Analog (theme-driven)
    case 54: // Shadow_Clock (theme-driven)
    case 55: // Shijian (theme-driven)
    case 56: // Skully (theme-driven)
    case 57: // Skykid_Watch (theme-driven)
    case 58: // Slacker (theme-driven)
    case 59: // SmartWatchy (theme-driven)
    case 60: // Spiral_Watchy (theme-driven)
    case 61: // Squarbital (theme-driven)
    case 62: // Squaro (theme-driven)
    case 63: // Star_Wars_Aurebesh (theme-driven)
    case 64: // StarryHorizon (theme-driven)
    case 65: // Stationary_Text (theme-driven)
    case 66: // Steps (theme-driven via BCD WatchyStep)
    case 67: // SW_Watchy (theme-driven)
    case 68: // Sundial (theme-driven)
    case 69: // Tetris (theme-driven)
    case 70: // The_Blob (theme-driven)
    case 71: // Triangle (theme-driven)
    case 72: // TypoStyle (theme-driven)
    case 73: // Watchy_Akira (theme-driven)
    case 74: // Watchy_PowerShell (theme-driven)
    case 75: // WatchySevenSegment (theme-driven)
    case 76: // X_marks_the_spot (theme-driven)
    case 5:  // Basic (SDK-based wrapper)
    case 37: // Mario (uses UiSDK::renderApp inside Watchy_Mario.cpp)
      return true;
    default:
      return false;
  }
}

// Theme helper implementations
uint16_t UiSDK::getWatchfaceBg(WatchfacePolarity polarity) {
  return paletteFor(polarity).bg;
}

uint16_t UiSDK::getWatchfaceFg(WatchfacePolarity polarity) {
  return paletteFor(polarity).fg;
}

bool UiSDK::shouldInvertBitmap(const Palette &palette) {
  // Invert when authored polarity disagrees with the live theme.
  return palette.bg != GxEPD_BLACK;
}

// Font helpers ---------------------------------------------------------------
const GFXfont *UiSDK::defaultFont() {
  initFontRegistryOnce();
  return gFontSlots[0];
}

const GFXfont *UiSDK::tinyFont5x7() {
  initFontRegistryOnce();
  return gTinyFont5x7;
}

const GFXfont *UiSDK::tinyFont6x8() {
  initFontRegistryOnce();
  return gTinyFont6x8;
}

const GFXfont *UiSDK::tinyMono6x8() {
  initFontRegistryOnce();
  return gTinyMono6x8;
}

bool UiSDK::registerFont(uint8_t slot, const GFXfont *font) {
  initFontRegistryOnce();
  if (slot >= kFontSlotCount) {
    return false;
  }
  gFontSlots[slot] = font;
  return true;
}

const GFXfont *UiSDK::getFont(const uint8_t slot) {
  initFontRegistryOnce();
  if (slot >= kFontSlotCount) {
    return nullptr;
  }
  return gFontSlots[slot];
}

bool UiSDK::buttonPressed(uint8_t pin, uint32_t debounceMs, bool activeLow) {
  DebounceSlot *slot = acquireDebounceSlot(pin);
  const bool rawPressed = activeLow ? (digitalRead(pin) == ACTIVE_LOW)
                                    : (digitalRead(pin) != ACTIVE_LOW);
  const uint32_t nowMs = millis();

  if (!rawPressed) {
    slot->latched = false;
    return false;
  }

  if (!slot->latched && (nowMs - slot->lastEventMs >= debounceMs)) {
    slot->latched = true;
    slot->lastEventMs = nowMs;
    return true;
  }

  return false;
}

void UiSDK::renderControlsRow(Watchy &watchy, const UIControlsRowLayout layout[4]) {
  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 
  watchy.display.setFont(&FreeMonoBold9pt7b);
  watchy.display.setTextColor(palette.fg);

  auto drawLabel = [&](int16_t x, int16_t y, const char *label, bool rightAlign) {
    if (label == nullptr) {
      return;
    }
    int16_t x1, y1;
    uint16_t w, h;
    watchy.display.getTextBounds(label, x, y, &x1, &y1, &w, &h);
    int16_t cursorX = rightAlign ? x - static_cast<int16_t>(w) : x;
    watchy.display.setCursor(cursorX, y);
    watchy.display.println(label);
  };

  const int16_t topY = 14;
  const int16_t bottomY = WatchyDisplay::HEIGHT - 6;

  drawLabel(4, topY, layout[0].label, false);
  drawLabel(WatchyDisplay::WIDTH - 4, topY, layout[1].label, true);
  drawLabel(4, bottomY, layout[2].label, false);
  drawLabel(WatchyDisplay::WIDTH - 4, bottomY, layout[3].label, true);


  // Read input first (always), then invoke callbacks if wired.
  // This keeps debounce/latch state consistent across screens, even when a
  // particular control is temporarily disabled (callback == nullptr).
  const bool back = buttonPressed(BACK_BTN_PIN);
  const bool up = buttonPressed(UP_BTN_PIN);
  const bool accept = buttonPressed(MENU_BTN_PIN);
  const bool down = buttonPressed(DOWN_BTN_PIN);

  if (layout[0].callback && back) {
    (watchy.*(layout[0].callback))();
  }
  if (layout[1].callback && up) {
    (watchy.*(layout[1].callback))();
  }
  if (layout[2].callback && accept) {
    (watchy.*(layout[2].callback))();
  }
  if (layout[3].callback && down) {
    (watchy.*(layout[3].callback))();
  }
}

namespace UiSDK {

void drawTextLine(WatchyGxDisplay &display,
                  const int16_t x,
                  const int16_t y,
                  const String &text,
                  const GFXfont *font,
                  const uint16_t color) {
  const GFXfont *useFont = font;
  if (useFont == nullptr) {
    useFont = UiSDK::defaultFont();
  }

  const bool useTiny6x8 = (useFont == gTinyFont6x8);
  const bool useTinyMono6x8 = (useFont == gTinyMono6x8);
  const bool useTiny5x7 = (useFont == gTinyFont5x7);

  if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
    drawTinyText(display, x, y, text, color);
    return;
  }

  display.setFont(useFont);
  display.setTextColor(color);
  display.setCursor(x, y);
  display.print(text);
}

void renderText(WatchyGxDisplay &display, const UITextSpec &spec) {
  int16_t x1, y1;
  uint16_t w, h;

  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 
  const uint16_t textColor = palette.fg; 
  const uint16_t backgroundColor = palette.bg;

  if(spec.fillBackground){
    display.drawRect(spec.x, spec.y - 8, spec.w > 0 ? spec.w : WatchyDisplay::WIDTH, spec.h > 0 ? spec.h : 16, backgroundColor); 
  }

  const GFXfont *font = spec.font;
  if (font == nullptr) {
    font = UiSDK::getFont(0);
  }
  if (font == nullptr) {
    font = UiSDK::defaultFont();
  }

  const bool useTiny6x8 = (font == gTinyFont6x8);
  const bool useTinyMono6x8 = (font == gTinyMono6x8);
  const bool useTiny5x7 = (font == gTinyFont5x7);

  
  if (spec.fillBackground) {
    display.getTextBounds(spec.text, spec.x, spec.y, &x1, &y1, &w, &h);
    int16_t bw = (spec.w > 0) ? spec.w : static_cast<int16_t>(w);
    int16_t bh = (spec.h > 0) ? spec.h : static_cast<int16_t>(h + 4);
    // Draw background box right behind text.
    display.fillRect(spec.x, spec.y - bh + 2, bw, bh, spec.fillBackground ? backgroundColor : textColor);
  }

  if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
    if (spec.fillBackground) {
      uint16_t len = spec.text.length();
      int16_t bw   = (spec.w > 0) ? spec.w : static_cast<int16_t>(len * 5);
      int16_t bh   = (spec.h > 0) ? spec.h : static_cast<int16_t>(8);
      display.fillRect(spec.x, spec.y - 7, bw, bh, spec.invert ? textColor : backgroundColor);
    }    
    drawTinyText(display, spec.x, spec.y, spec.text, spec.invert ? backgroundColor : textColor);
    return;
  }
  display.setFont(font);
  display.setTextColor(spec.invert ? backgroundColor : textColor);
  display.setCursor(spec.x, spec.y);
  display.print(spec.text);

}

void renderImage(WatchyGxDisplay &display, const UIImageSpec &spec) {
  if (spec.bitmap == nullptr) {
    return;
  }  
  drawBitmapUnaligned(display, spec);
}

void renderMenu(WatchyGxDisplay &display, const UIMenuSpec &spec) {
  if (spec.font != nullptr) {
    display.setFont(spec.font);
  }

  const Palette palette = paletteFor(BASE_POLARITY);

  uint8_t first  = spec.startIndex;
  uint8_t maxRow = (spec.visibleCount == 0) ? spec.itemCount : spec.visibleCount;

  for (uint8_t row = 0; row < maxRow; ++row) {
    uint8_t itemIndex = first + row;
    if (itemIndex >= spec.itemCount) {
      break;
    }

    const int16_t yPos = spec.y + spec.itemHeight * row;
    const String &label = spec.items[itemIndex].label;
    if (label.length() == 0) {
      continue;
    }

    const bool selected = (itemIndex == spec.selectedIndex);
    const uint16_t rowBg = selected ? palette.accentBg : palette.bg;
    const uint16_t rowFg = selected ? palette.accentFg : palette.fg;

    // Limit fill to the row area to avoid repainting the entire screen per item.
    const int16_t rowHeight = (spec.h > 0) ? spec.h : spec.itemHeight;
    display.fillRect(
      0,
      yPos - spec.itemHeight + 2,
      spec.w > 0 ? spec.w : WatchyDisplay::WIDTH,
      rowHeight,
      rowBg
    );

    display.setTextColor(rowFg);
    display.setCursor(spec.x, yPos);
    display.println(label);

  #if defined(ESP8266) || defined(ESP32)
    yield(); // avoid watchdog resets on large menus
  #endif
  }

  const bool hasAbove = (spec.startIndex > 0);
  const bool hasBelow = (spec.startIndex + maxRow < spec.itemCount);
  const int16_t rightX = WatchyDisplay::WIDTH - 6;

  if (hasAbove) {
    const int16_t topY = spec.y - spec.itemHeight + 10;
    display.fillTriangle(rightX - 4, topY + 4,
                         rightX + 4, topY + 4,
                         rightX,     topY - 2,
                         palette.fg);
  }

  if (hasBelow) {
    const int16_t bottomY = spec.y + spec.itemHeight * ((maxRow > 0 ? maxRow : 1) - 1) + 10;
    display.fillTriangle(rightX - 4, bottomY - 4,
                         rightX + 4, bottomY - 4,
                         rightX,     bottomY + 2,
                         palette.fg);
  }
}

void renderButton(WatchyGxDisplay &display, const UIButtonSpec &spec) {
  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 
  uint16_t fg =  palette.fg; 
  uint16_t bg = palette.bg; 

  if (spec.selected) {
    bg = palette.accentBg;
    fg = palette.accentFg;
  }

  display.fillRect(spec.x, spec.y - spec.h, spec.w, spec.h, bg);

  if (spec.font != nullptr) {
    display.setFont(spec.font);
  }

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(spec.label, spec.x, spec.y, &x1, &y1, &w, &h);
  const int16_t textX = spec.x + (spec.w - static_cast<int16_t>(w)) / 2;
  const int16_t textY = spec.y - (spec.h - static_cast<int16_t>(h)) / 2;

  display.setTextColor(fg);
  display.setCursor(textX, textY);
  display.println(spec.label);
}

void renderCheckbox(WatchyGxDisplay &display, const UICheckboxSpec &spec) {
  if (spec.font != nullptr) {
    display.setFont(spec.font);
  }

  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 
  const uint16_t textColor = palette.fg;
  const uint16_t backgroundColor = palette.bg;

  String fullText = (spec.checked ? "[x] " : "[ ] ") + spec.label;

  if (spec.fillBackground) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(fullText, spec.x, spec.y, &x1, &y1, &w, &h);
    display.fillRect(spec.x, y1 - 2, w, h + 4, backgroundColor);
  }

  display.setTextColor(textColor);
  display.setCursor(spec.x, spec.y);
  display.print(fullText);
}

void renderScrollableText(WatchyGxDisplay &display, const UIScrollableTextSpec &spec) {
  const GFXfont *font = spec.font;
  if (font == nullptr) {
    font = UiSDK::getFont(0);
  }
  if (font == nullptr) {
    font = UiSDK::defaultFont();
  }

  const bool useTiny6x8 = (font == gTinyFont6x8);
  const bool useTinyMono6x8 = (font == gTinyMono6x8);
  const bool useTiny5x7 = (font == gTinyFont5x7);
  if (!useTiny6x8 && !useTinyMono6x8 && !useTiny5x7 && font != nullptr) {
    display.setFont(font);
  }

  Palette palette = BASE_POLARITY == WatchfacePolarity::WhiteOnBlack ? paletteFor(WatchfacePolarity::WhiteOnBlack) : paletteFor(WatchfacePolarity::BlackOnWhite); 

  const int16_t padding = 6;
  const int16_t contentX = spec.x + padding;
  const int16_t contentY = spec.y + padding;
  const int16_t contentW = spec.w - padding * 2;
  const int16_t contentH = spec.h - padding * 2;

  if (spec.fillBackground) {
    display.fillRect(spec.x, spec.y, spec.w, spec.h, palette.bg);
  }

  display.setTextColor(palette.fg);

  uint16_t computedLineHeight = spec.lineHeight;
  if (computedLineHeight == 0) {
    if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
      computedLineHeight = 10; // tiny font baseline + spacing
    } else {
      int16_t bx, by;
      uint16_t bw, bh;
      display.getTextBounds("Ag", contentX, contentY, &bx, &by, &bw, &bh);
      computedLineHeight = static_cast<uint16_t>(bh + 4); // add breathing room
    }
  }

  const String *textPtr = (spec.textRef != nullptr) ? spec.textRef : &spec.text;
  const char *cstr = textPtr->c_str();

  const uint16_t startLine = spec.firstLine;
  const uint16_t endLine = spec.firstLine + spec.maxLines;

  const int16_t tinyCharW = 5; // must match drawTinyText() advance

  const auto measureTextWidth = [&](const String &s, int16_t y) -> int16_t {
    if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
      return static_cast<int16_t>(s.length()) * tinyCharW;
    }
    int16_t bx, by;
    uint16_t bw, bh;
    display.getTextBounds(s, 0, y, &bx, &by, &bw, &bh);
    return static_cast<int16_t>(bw);
  };

  uint16_t currentLine = 0;
  int16_t drawY = contentY;
  const char *lineStart = cstr;
  bool hasBelow = false;

  // Wrapping is opt-in because some apps (TextBrowser) rely on a stable
  // 1:1 mapping between source lines and rendered lines.
  if (!spec.wrapLongLines) {
    uint16_t totalLines = 0;
    if (cstr[0] != '\0') {
      totalLines = 1;
      for (const char *p = cstr; *p; ++p) {
        if (*p == '\n') {
          ++totalLines;
        }
      }
    }

    while (lineStart && *lineStart) {
      const char *newline = strchr(lineStart, '\n');
      String line;
      if (newline) {
        line.reserve(static_cast<uint16_t>(newline - lineStart + 1));
        for (const char *p = lineStart; p < newline; ++p) {
          line += *p;
        }
      } else {
        line = lineStart;
      }

      if (currentLine >= startLine && currentLine < endLine) {
        int16_t drawX = contentX;
        if (spec.centered) {
          if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
            const int16_t approxWidth = static_cast<int16_t>(line.length()) * tinyCharW;
            drawX = contentX + (contentW - approxWidth) / 2;
          } else {
            int16_t bx, by;
            uint16_t bw, bh;
            display.getTextBounds(line, contentX, drawY, &bx, &by, &bw, &bh);
            drawX = contentX + (contentW - static_cast<int16_t>(bw)) / 2;
          }
        }

        if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
          drawTinyText(display, drawX, drawY, line, palette.fg);
        } else {
          display.setCursor(drawX, drawY);
          display.print(line);
        }
        drawY += computedLineHeight;
        if (drawY > contentY + contentH) {
          break;
        }
      }

      if (!newline) {
        break;
      }
      lineStart = newline + 1;
      ++currentLine;
    }

    hasBelow = (endLine < totalLines);
  } else {
    // Wrap long lines into multiple visual lines.
    const int16_t charW = (useTiny6x8 || useTinyMono6x8 || useTiny5x7) ? tinyCharW : 6;
    const uint16_t maxChars = (contentW > 0) ? static_cast<uint16_t>(contentW / charW) : 1;
    const uint16_t safeMaxChars = (maxChars < 1) ? 1 : maxChars;

    // Prefer wrapping at the nearest space to the *right* of the hard limit,
    // within a small lookahead window. Falls back to a space to the left, then
    // finally a hard cut.
    const size_t kWrapLookaheadChars = 12;

    const auto computeSegLen = [&](const char *line, size_t lineLen, size_t startPos) -> size_t {
      const size_t remaining = (lineLen > startPos) ? (lineLen - startPos) : 0;
      if (remaining <= safeMaxChars) {
        return remaining;
      }

      const size_t base = startPos + safeMaxChars;

      // Look right for the first space.
      for (size_t i = base; i < lineLen && i < base + kWrapLookaheadChars; ++i) {
        if (line[i] == ' ') {
          const size_t len = i - startPos;
          return (len > 0) ? len : safeMaxChars;
        }
      }

      // Otherwise, look left for a space.
      for (size_t i = base; i > startPos; --i) {
        if (line[i - 1] == ' ') {
          const size_t len = (i - 1) - startPos;
          return (len > 0) ? len : safeMaxChars;
        }
      }

      return safeMaxChars;
    };

    while (lineStart && *lineStart) {
      const char *newline = strchr(lineStart, '\n');
      const char *lineEnd = newline ? newline : (lineStart + strlen(lineStart));
      const size_t rawLen = static_cast<size_t>(lineEnd - lineStart);

      size_t pos = 0;
      uint8_t segIdx = 0;

      // Empty source line still consumes one visual line.
      if (rawLen == 0) {
        if (currentLine >= startLine && currentLine < endLine) {
          drawY += computedLineHeight;
        }
        ++currentLine;
      } else {
        while (pos < rawLen) {
          // Skip leading spaces on continuation lines.
          if (segIdx > 0) {
            while (pos < rawLen && lineStart[pos] == ' ') {
              ++pos;
            }
            if (pos >= rawLen) {
              break;
            }
          }

          const size_t segLen = computeSegLen(lineStart, rawLen, pos);

          if (currentLine >= startLine && currentLine < endLine) {
            String piece;
            if (segLen > 0) {
              piece.reserve(static_cast<uint16_t>(segLen + 1));
              piece.concat(lineStart + pos, static_cast<unsigned int>(segLen));
            }

            int16_t drawX = contentX;

            // Continuation lines can be right-aligned.
            if (segIdx > 0 && spec.wrapContinuationAlignRight) {
              const int16_t pieceW = measureTextWidth(piece, drawY);
              drawX = contentX + (contentW - pieceW);
              if (drawX < contentX) drawX = contentX;
            } else if (spec.centered) {
              const int16_t pieceW = measureTextWidth(piece, drawY);
              drawX = contentX + (contentW - pieceW) / 2;
            }

            if (useTiny6x8 || useTinyMono6x8 || useTiny5x7) {
              drawTinyText(display, drawX, drawY, piece, palette.fg);
            } else {
              display.setCursor(drawX, drawY);
              display.print(piece);
            }

            drawY += computedLineHeight;
            if (drawY > contentY + contentH) {
              // We ran out of vertical space; if there is more content than what
              // we just drew, show the below arrow.
              const bool moreInThisLine = (pos + segLen < rawLen);
              if (moreInThisLine || newline != nullptr) {
                hasBelow = true;
              }
              currentLine = endLine; // stop
              break;
            }
          }

          ++currentLine;
          ++segIdx;
          pos += segLen;

          if (currentLine >= endLine) {
            // If anything remains after the last visible line, signal below arrow.
            const bool moreInThisLine = (pos < rawLen);
            const bool moreLines = (newline != nullptr && *(newline + 1) != '\0');
            const bool trailingEmptyLine = (newline != nullptr && *(newline + 1) == '\0');
            if (moreInThisLine || moreLines || trailingEmptyLine) {
              hasBelow = true;
            }
            break;
          }
        }
      }

      if (currentLine >= endLine) {
        break;
      }

      if (!newline) {
        break;
      }
      lineStart = newline + 1;
    }
  }

  const bool hasAbove = (startLine > 0);
  const int16_t rightX = spec.x + spec.w - 6;

  if (hasAbove) {
    const int16_t topY = spec.y + 8;
    display.fillTriangle(rightX - 4, topY + 4,
                         rightX + 4, topY + 4,
                         rightX,     topY - 2,
                         palette.fg);
  }

  if (hasBelow) {
    const int16_t bottomY = spec.y + spec.h - 8;
    display.fillTriangle(rightX - 4, bottomY - 4,
                         rightX + 4, bottomY - 4,
                         rightX,     bottomY + 2,
                         palette.fg);
  }
}

uint16_t countScrollableTextLines(const UIScrollableTextSpec &spec) {
  const int16_t padding = 6;
  const int16_t contentW = spec.w - padding * 2;

  const GFXfont *font = spec.font;
  if (font == nullptr) {
    font = UiSDK::getFont(0);
  }
  if (font == nullptr) {
    font = UiSDK::defaultFont();
  }

  const bool useTiny = (font == gTinyFont6x8) || (font == gTinyMono6x8) || (font == gTinyFont5x7);
  const int16_t charW = useTiny ? 5 : 6; // tiny must match drawTinyText() advance

  const String *textPtr = (spec.textRef != nullptr) ? spec.textRef : &spec.text;
  const char *cstr = textPtr->c_str();
  if (cstr[0] == '\0') {
    return 0;
  }

  // Default behavior: newline-delimited.
  if (!spec.wrapLongLines) {
    uint16_t total = 1;
    for (const char *p = cstr; *p; ++p) {
      if (*p == '\n') {
        ++total;
      }
    }
    return total;
  }

  const uint16_t maxChars = (contentW > 0) ? static_cast<uint16_t>(contentW / charW) : 1;
  const uint16_t safeMaxChars = (maxChars < 1) ? 1 : maxChars;

  const size_t kWrapLookaheadChars = 12;
  const auto computeSegLen = [&](const char *line, size_t lineLen, size_t startPos) -> size_t {
    const size_t remaining = (lineLen > startPos) ? (lineLen - startPos) : 0;
    if (remaining <= safeMaxChars) {
      return remaining;
    }

    const size_t base = startPos + safeMaxChars;

    for (size_t i = base; i < lineLen && i < base + kWrapLookaheadChars; ++i) {
      if (line[i] == ' ') {
        const size_t len = i - startPos;
        return (len > 0) ? len : safeMaxChars;
      }
    }

    for (size_t i = base; i > startPos; --i) {
      if (line[i - 1] == ' ') {
        const size_t len = (i - 1) - startPos;
        return (len > 0) ? len : safeMaxChars;
      }
    }

    return safeMaxChars;
  };

  uint16_t total = 0;
  const char *p = cstr;
  while (p && *p) {
    const char *newline = strchr(p, '\n');
    const char *end = newline ? newline : (p + strlen(p));
    const size_t len = static_cast<size_t>(end - p);

    if (len == 0) {
      total += 1;
    } else {
      size_t pos = 0;
      uint8_t segIdx = 0;
      while (pos < len) {
        if (segIdx > 0) {
          while (pos < len && p[pos] == ' ') {
            ++pos;
          }
          if (pos >= len) {
            break;
          }
        }
        const size_t segLen = computeSegLen(p, len, pos);
        pos += segLen;
        ++segIdx;
        ++total;
      }
      if (segIdx == 0) {
        total += 1;
      }
    }

    if (!newline) {
      break;
    }
    p = newline + 1;
    // If there's a trailing newline, it represents an extra empty line.
    if (*p == '\0') {
      total += 1;
      break;
    }
  }

  return total;
}

void renderCallback(WatchyGxDisplay &display, const UICallbackSpec &spec) {
  if (spec.draw) {
    spec.draw(display, spec.userData);
  }
}

void renderWatchfaceSpec(Watchy &watchy, const UIAppSpec &app) {
  watchy.display.epd2.setDarkBorder(app.chrome.darkBorder);
  watchy.display.setFullWindow();
#if defined(ESP8266) || defined(ESP32)
  yield();
#endif

  for (uint8_t i = 0; i < app.callbackCount; ++i) {
    renderCallback(watchy.display, app.callbacks[i]);
  }

  for (uint8_t i = 0; i < app.imageCount; ++i) {
    renderImage(watchy.display, app.images[i]);
  }

  for (uint8_t i = 0; i < app.menuCount; ++i) {
    renderMenu(watchy.display, app.menus[i]);
  }

#if defined(ESP8266) || defined(ESP32)
  yield();
#endif

  for (uint8_t i = 0; i < app.buttonCount; ++i) {
    renderButton(watchy.display, app.buttons[i]);
  }

  for (uint8_t i = 0; i < app.checkboxCount; ++i) {
    renderCheckbox(watchy.display, app.checkboxes[i]);
  }

  for (uint8_t i = 0; i < app.scrollTextCount; ++i) {
    renderScrollableText(watchy.display, app.scrollTexts[i]);
  }

  for (uint8_t i = 0; i < app.textCount; ++i) {
    renderText(watchy.display, app.texts[i]);
  }
}

void renderApp(Watchy &watchy, const UIAppSpec &app) {
  // Apply shared chrome (optional). Most apps/OS apps want a consistent
  // full-screen setup + controls row; watchfaces typically leave this disabled.
  watchy.display.epd2.setDarkBorder(app.chrome.darkBorder);
  initScreen(watchy.display);
#if defined(ESP8266) || defined(ESP32)
  yield();
#endif
  // Order is intentional: callbacks first (for custom backgrounds), then images,
  // menus, buttons, checkboxes, scrollable text, and finally text.  
  renderControlsRow(watchy, app.controls);
#if defined(ESP8266) || defined(ESP32)
  yield();
#endif
  
  for (uint8_t i = 0; i < app.callbackCount; ++i) {
    renderCallback(watchy.display, app.callbacks[i]);
  }

  for (uint8_t i = 0; i < app.imageCount; ++i) {
    renderImage(watchy.display, app.images[i]);
  }

  for (uint8_t i = 0; i < app.menuCount; ++i) {
    renderMenu(watchy.display, app.menus[i]);
  }

#if defined(ESP8266) || defined(ESP32)
  yield();
#endif

  for (uint8_t i = 0; i < app.buttonCount; ++i) {
    renderButton(watchy.display, app.buttons[i]);
  }

  for (uint8_t i = 0; i < app.checkboxCount; ++i) {
    renderCheckbox(watchy.display, app.checkboxes[i]);
  }

  for (uint8_t i = 0; i < app.scrollTextCount; ++i) {
    renderScrollableText(watchy.display, app.scrollTexts[i]);
  }

  for (uint8_t i = 0; i < app.textCount; ++i) {
    renderText(watchy.display, app.texts[i]);
  }

  watchy.display.display(true); // always partial to avoid full refresh on transitions
}

void initScreen(WatchyGxDisplay &display) {
  setPolarity(BASE_POLARITY);
  display.setFullWindow();
  display.clearScreen(getWatchfaceBg(getThemePolarity()));
  display.setTextColor(getWatchfaceFg(getThemePolarity()));  
  display.fillScreen(getWatchfaceBg(getThemePolarity()));
}
} // namespace UiSDK