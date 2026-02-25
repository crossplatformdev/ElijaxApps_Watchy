#include "WatchyUi.h"
#include <Preferences.h>
#include <string.h>

namespace WatchyUi {

constexpr int16_t Theme::screenMargin;
constexpr int16_t Theme::titleBaseline;
constexpr int16_t Theme::contentBaseline;
constexpr int16_t Theme::listFirstBaseline;
constexpr int16_t Theme::footerBaseline;
constexpr int16_t Theme::listRowHeight;
constexpr uint8_t Theme::listVisibleRows;

namespace {

uint8_t previousButtons = 0;
RTC_DATA_ATTR bool themeLoaded = false;

constexpr uint8_t selectBit = 1U << 0;
constexpr uint8_t backBit = 1U << 1;
constexpr uint8_t upBit = 1U << 2;
constexpr uint8_t downBit = 1U << 3;
constexpr Bounds messageIconBounds{80, 58, 41, 41};
constexpr int16_t messageBodyTop =
  messageIconBounds.y + messageIconBounds.height + 7;

uint8_t readButtons() {
  uint8_t buttons = 0;
  if (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW) buttons |= selectBit;
  if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) buttons |= backBit;
  if (digitalRead(UP_BTN_PIN) == ACTIVE_LOW) buttons |= upBit;
  if (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW) buttons |= downBit;
  return buttons;
}

struct LabelArray {
  const char *const *labels;
};

const char *labelFromArray(uint8_t index, const void *context) {
  return static_cast<const LabelArray *>(context)->labels[index];
}

void printClipped(const char *text, uint8_t maximumCharacters) {
  for (uint8_t index = 0;
       text[index] != '\0' && index < maximumCharacters; index++) {
    Watchy::display.print(text[index]);
  }
}

int16_t centeredTextX(const char *text, uint8_t textSize) {
  int16_t width = strlen(text) * 6 * textSize;
  return max<int16_t>(Theme::screenMargin, (DISPLAY_WIDTH - width) / 2);
}

void drawMessageIcon(MessageKind kind) {
  const uint16_t color = Theme::foreground();
  const char *symbol = "i";
  if (kind == MessageKind::SUCCESS) symbol = "+";
  else if (kind == MessageKind::WARNING) symbol = "!";
  else if (kind == MessageKind::ERROR) symbol = "x";
  else if (kind == MessageKind::LOADING) symbol = "...";
  else if (kind == MessageKind::EMPTY) symbol = "-";
  Canvas::circle(messageIconBounds, color);
  Canvas::centeredText(messageIconBounds, symbol, 2, color);
}

} // namespace

Bounds Bounds::inset(int16_t amount) const {
  if (amount <= 0) return *this;
  int16_t horizontal = min<int16_t>(amount, max<int16_t>(0, width / 2));
  int16_t vertical = min<int16_t>(amount, max<int16_t>(0, height / 2));
  return Bounds{static_cast<int16_t>(x + horizontal),
                static_cast<int16_t>(y + vertical),
                max<int16_t>(0, width - horizontal * 2),
                max<int16_t>(0, height - vertical * 2)};
}

void Canvas::outline(const Bounds &bounds, uint16_t color) {
  if (bounds.width <= 0 || bounds.height <= 0) return;
  Watchy::display.drawRect(bounds.x, bounds.y, bounds.width, bounds.height,
                           color);
}

void Canvas::circle(const Bounds &bounds, uint16_t color, bool filled) {
  if (bounds.width <= 0 || bounds.height <= 0) return;
  int16_t diameter = min(bounds.width, bounds.height);
  int16_t radius = (diameter - 1) / 2;
  int16_t centerX = bounds.x + (bounds.width - 1) / 2;
  int16_t centerY = bounds.y + (bounds.height - 1) / 2;
  if (filled) Watchy::display.fillCircle(centerX, centerY, radius, color);
  else Watchy::display.drawCircle(centerX, centerY, radius, color);
}

void Canvas::centeredText(const Bounds &bounds, const char *text,
                          uint16_t color) {
  if (bounds.width <= 0 || bounds.height <= 0 || text == nullptr ||
      text[0] == '\0') {
    return;
  }
  Watchy::display.setTextColor(color);
  int16_t textX;
  int16_t textY;
  uint16_t textWidth;
  uint16_t textHeight;
  Watchy::display.getTextBounds(text, 0, 0, &textX, &textY,
                                &textWidth, &textHeight);
  int16_t cursorX = bounds.x +
                    (bounds.width - static_cast<int16_t>(textWidth)) / 2 -
                    textX;
  int16_t cursorY = bounds.y +
                    (bounds.height - static_cast<int16_t>(textHeight)) / 2 -
                    textY;
  Watchy::display.setCursor(cursorX, cursorY);
  Watchy::display.print(text);
}

void Canvas::centeredText(const Bounds &bounds, const char *text,
                          uint8_t textSize, uint16_t color) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(max<uint8_t>(1, textSize));
  centeredText(bounds, text, color);
}

uint16_t Theme::background() {
  return DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;
}

uint16_t Theme::foreground() {
  return DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
}

void Theme::load() {
  if (themeLoaded) {
    return;
  }
  Preferences preferences;
  if (preferences.begin("watchy-ui", true)) {
    DARKMODE = preferences.getBool("dark", DARKMODE);
    preferences.end();
  }
  themeLoaded = true;
}

bool Theme::setDark(bool dark) {
  load();
  Preferences preferences;
  if (!preferences.begin("watchy-ui", false)) {
    return false;
  }
  bool saved = preferences.putBool("dark", dark) == sizeof(bool);
  preferences.end();
  if (saved) {
    DARKMODE = dark;
  }
  return saved;
}

#ifdef WATCHY_DETERMINISTIC_GALLERY
void Theme::useGalleryTheme(bool dark) {
  DARKMODE = dark;
  themeLoaded = true;
}
#endif

void Screen::beginCanvas() {
  Theme::load();
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(Theme::background());
  Watchy::display.setTextColor(Theme::foreground());
  Watchy::display.setTextSize(1);
  Watchy::display.setTextWrap(false);
}

void Screen::begin(const char *title) {
  beginCanvas();
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setCursor(0, Theme::titleBaseline);
  Watchy::display.println(title);
}

void Screen::present(int nextGuiState) {
  Watchy::display.display(
      !Watchy::display.epd2.initialFullRefreshPending());
  guiState = nextGuiState;
}

void Widget::bodyText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(Theme::foreground());
  Watchy::display.setCursor(x, y);
}

void Widget::separator(int16_t y) {
  Watchy::display.drawFastHLine(Theme::screenMargin, y,
                                DISPLAY_WIDTH - Theme::screenMargin * 2,
                                Theme::foreground());
}

void Widget::footer(const char *commands) {
  if (commands == nullptr || commands[0] == '\0') {
    return;
  }
  const uint16_t color = Theme::foreground();
  Watchy::display.drawFastHLine(Theme::screenMargin, 178,
                                DISPLAY_WIDTH - Theme::screenMargin * 2,
                                color);
  bodyText(Theme::screenMargin, Theme::footerBaseline);
  Watchy::display.print(commands);
}

void Widget::paragraph(const char *text, int16_t x, int16_t y,
                       uint8_t columns, uint8_t maximumLines,
                       uint8_t lineHeight) {
  bodyText(x, y);
  uint8_t column = 0;
  uint8_t line = 0;
  for (const char *cursor = text;
       *cursor != '\0' && line < maximumLines; cursor++) {
    if (*cursor == '\r') {
      continue;
    }
    if (*cursor == '\n' || column >= columns) {
      line++;
      column = 0;
      if (line >= maximumLines) {
        break;
      }
      Watchy::display.setCursor(x, y + line * lineHeight);
      if (*cursor == '\n') {
        continue;
      }
    }
    Watchy::display.print(*cursor);
    column++;
  }
}

void Widget::progress(float value, int16_t y) {
  value = constrain(value, 0.0f, 1.0f);
  const uint16_t color = Theme::foreground();
  Canvas::outline(Bounds{Theme::screenMargin, y,
                         DISPLAY_WIDTH - Theme::screenMargin * 2, 17}, color);
  Watchy::display.fillRect(Theme::screenMargin + 3, y + 3,
                           static_cast<int16_t>(
                               (DISPLAY_WIDTH - Theme::screenMargin * 2 - 6) *
                               value),
                           11, color);
}

void Widget::checkbox(const char *label, bool checked, int16_t y,
                      bool focused, bool enabled) {
  const uint16_t foreground = Theme::foreground();
  const uint16_t background = Theme::background();
  if (focused) {
    Watchy::display.fillRect(Theme::screenMargin, y - 6,
                             DISPLAY_WIDTH - Theme::screenMargin * 2, 20,
                             foreground);
  }
  uint16_t color = focused ? background : foreground;
  const Bounds controlBounds{8, static_cast<int16_t>(y - 2), 12, 12};
  Canvas::outline(controlBounds, color);
  if (checked) {
    Bounds markBounds = controlBounds.inset(3);
    Watchy::display.fillRect(markBounds.x, markBounds.y, markBounds.width,
                             markBounds.height, color);
  }
  bodyText(28, y);
  Watchy::display.setTextColor(color);
  Watchy::display.print(enabled ? label : "[DISABLED] ");
  if (!enabled) Watchy::display.print(label);
}

void Widget::radio(const char *label, bool selected, int16_t y,
                   bool focused, bool enabled) {
  const uint16_t foreground = Theme::foreground();
  const uint16_t background = Theme::background();
  if (focused) {
    Watchy::display.fillRect(Theme::screenMargin, y - 6,
                             DISPLAY_WIDTH - Theme::screenMargin * 2, 20,
                             foreground);
  }
  uint16_t color = focused ? background : foreground;
  const Bounds controlBounds{8, static_cast<int16_t>(y - 2), 13, 13};
  Canvas::circle(controlBounds, color);
  if (selected) Canvas::circle(controlBounds.inset(3), color, true);
  bodyText(28, y);
  Watchy::display.setTextColor(color);
  if (!enabled) Watchy::display.print("[DISABLED] ");
  Watchy::display.print(label);
}

void Widget::toggle(bool enabled, int16_t x, int16_t y, bool focused) {
  const uint16_t foreground = Theme::foreground();
  const uint16_t background = Theme::background();
  uint16_t color = focused ? background : foreground;
  if (focused) {
    Watchy::display.fillRect(x - 3, y - 3, 36, 22, foreground);
  }
  Watchy::display.drawRoundRect(x, y, 30, 16, 8, color);
  Watchy::display.fillCircle(enabled ? x + 22 : x + 8, y + 8, 5, color);
}

void ValueView::draw(const ValueModel &model) {
  Screen::begin(model.title);
  Widget::separator();
  Watchy::display.setFont();
  uint8_t valueSize = strlen(model.value) > 8 ? 2 : 4;
  Watchy::display.setTextSize(valueSize);
  Watchy::display.setCursor(centeredTextX(model.value, valueSize), 92);
  Watchy::display.print(model.value);
  if (model.status != nullptr && model.status[0] != '\0') {
    uint8_t statusSize = strlen(model.status) > 15 ? 1 : 2;
    Watchy::display.setTextSize(statusSize);
    Watchy::display.setCursor(centeredTextX(model.status, statusSize), 126);
    Watchy::display.print(model.status);
  }
  if (model.detail != nullptr && model.detail[0] != '\0') {
    Widget::paragraph(model.detail, Theme::screenMargin, 149, 32, 3);
  }
  Widget::footer(model.footer);
}

void Feedback::showMessage(const char *title, const char *message,
                           MessageKind kind, const char *footer) {
  Screen::begin(title);
  Widget::separator();
  drawMessageIcon(kind);
  Widget::paragraph(message, Theme::screenMargin, messageBodyTop, 32, 7, 10);
  Widget::footer(footer);
  Screen::present();
}

bool Feedback::confirm(const char *title, const char *message) {
  showMessage(title, message, MessageKind::WARNING,
              "SELECT CONFIRM     BACK CANCEL");
  Input::begin();
  while (true) {
    Event event = Input::wait();
    if (event == Event::SELECT) return true;
    if (event == Event::BACK) return false;
  }
}

void Feedback::toast(const char *message) {
  const uint16_t foreground = Theme::foreground();
  const uint16_t background = Theme::background();
  Watchy::display.setPartialWindow(4, 158, 192, 38);
  Watchy::display.fillRect(4, 158, 192, 38, foreground);
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(background);
  Watchy::display.setCursor(10, 181);
  Watchy::display.print(message);
  Watchy::display.display(true);
  Watchy::display.setFullWindow();
}

void Input::begin() {
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);
  previousButtons = readButtons();
}

Event Input::poll() {
  uint8_t currentButtons = readButtons();
  uint8_t pressedButtons = currentButtons & ~previousButtons;
  previousButtons = currentButtons;
  if (pressedButtons == 0) {
    return Event::NONE;
  }

  delay(15);
  currentButtons = readButtons();
  pressedButtons &= currentButtons;
  previousButtons = currentButtons;

  if (pressedButtons & backBit) return Event::BACK;
  if (pressedButtons & selectBit) return Event::SELECT;
  if (pressedButtons & upBit) return Event::UP;
  if (pressedButtons & downBit) return Event::DOWN;
  return Event::NONE;
}

Event Input::wait(uint32_t timeoutMs) {
  uint32_t startedAt = millis();
  while (timeoutMs == UINT32_MAX || millis() - startedAt < timeoutMs) {
    Event event = poll();
    if (event != Event::NONE) {
      return event;
    }
    delay(10);
  }
  return Event::NONE;
}

bool Input::pressed(Event event) {
  uint8_t buttons = readButtons();
  switch (event) {
  case Event::SELECT: return buttons & selectBit;
  case Event::BACK: return buttons & backBit;
  case Event::UP: return buttons & upBit;
  case Event::DOWN: return buttons & downBit;
  default: return false;
  }
}

void Input::waitForRelease(Event event) {
  while (pressed(event)) {
    delay(10);
  }
  previousButtons = readButtons();
}

void ListView::draw(const ListModel &model) {
  Screen::begin(model.title);
  const uint16_t background = Theme::background();
  const uint16_t foreground = Theme::foreground();
  Watchy::display.drawFastHLine(Theme::screenMargin, 23,
                                DISPLAY_WIDTH - Theme::screenMargin * 2,
                                foreground);

  uint8_t visibleRows = constrain(model.visibleRows, 1,
                                  Theme::listVisibleRows);
  uint8_t itemCount = model.itemCount;
  uint8_t selected = itemCount == 0 ? 0 : model.selectedIndex % itemCount;
  uint8_t firstItem = selected >= visibleRows
                          ? selected - visibleRows + 1
                          : 0;
  uint8_t lastItem = min<int>(firstItem + visibleRows, itemCount);

  for (uint8_t item = firstItem; item < lastItem; item++) {
    int16_t baseline = Theme::listFirstBaseline +
                       (item - firstItem) * Theme::listRowHeight;
    bool isSelected = item == selected;
    if (isSelected) {
      Watchy::display.fillRect(Theme::screenMargin, baseline - 16,
                               DISPLAY_WIDTH - Theme::screenMargin * 2,
                               Theme::listRowHeight - 1, foreground);
    }
    Watchy::display.setTextColor(isSelected ? background : foreground);
    int16_t labelX = 10;
    if (model.activeIndex >= 0) {
      int16_t centerY = baseline - 6;
        const Bounds indicatorBounds{
          8, static_cast<int16_t>(centerY - 4), 9, 9};
      Canvas::circle(indicatorBounds,
                     isSelected ? background : foreground);
      if (item == model.activeIndex) {
        Canvas::circle(indicatorBounds.inset(2),
                       isSelected ? background : foreground, true);
      }
      labelX = 24;
    }
    Watchy::display.setCursor(labelX, baseline);
    printClipped(model.labelAt(item, model.context),
                 model.detailAt == nullptr ? 26 : 14);
    if (model.detailAt != nullptr) {
      Watchy::display.setCursor(104, baseline);
      printClipped(model.detailAt(item, model.context), 14);
    }
    if (model.showDisclosure) {
      Watchy::display.setCursor(183, baseline);
      Watchy::display.print('>');
    }
  }

  Watchy::display.setTextColor(foreground);
  if (firstItem > 0) {
    Watchy::display.fillTriangle(193, 25, 188, 31, 198, 31, foreground);
  }
  if (lastItem < itemCount) {
    Watchy::display.fillTriangle(188, 169, 198, 169, 193, 175, foreground);
  }

  if (model.footer != nullptr && model.footer[0] != '\0') {
    Watchy::display.drawFastHLine(Theme::screenMargin, 178,
                                  DISPLAY_WIDTH - Theme::screenMargin * 2,
                                  foreground);
    Watchy::display.setFont();
    Watchy::display.setTextSize(1);
    Watchy::display.setCursor(Theme::screenMargin, Theme::footerBaseline);
    Watchy::display.print(model.footer);
  }
}

void ListView::draw(const char *title, const char *const labels[],
                    uint8_t itemCount, uint8_t selectedIndex,
                    const char *footer, int16_t activeIndex,
                    uint8_t visibleRows) {
  LabelArray context{labels};
  ListModel model{title, labelFromArray, nullptr, &context, footer, itemCount,
                  selectedIndex, visibleRows, activeIndex, false};
  draw(model);
}

uint8_t ListView::previous(uint8_t selectedIndex, uint8_t itemCount) {
  return itemCount == 0 ? 0 : (selectedIndex + itemCount - 1) % itemCount;
}

uint8_t ListView::next(uint8_t selectedIndex, uint8_t itemCount) {
  return itemCount == 0 ? 0 : (selectedIndex + 1) % itemCount;
}

int32_t Selector::step(int32_t value, int32_t amount, int32_t minimum,
                       int32_t maximum, bool wrap) {
  if (minimum > maximum) {
    return minimum;
  }
  int64_t candidate = static_cast<int64_t>(value) + amount;
  if (!wrap) {
    return constrain(candidate, static_cast<int64_t>(minimum),
                     static_cast<int64_t>(maximum));
  }
  int64_t range = static_cast<int64_t>(maximum) - minimum + 1;
  int64_t offset = (candidate - minimum) % range;
  if (offset < 0) offset += range;
  return minimum + offset;
}

void Selector::formatTime(char output[6], uint8_t hour, uint8_t minute) {
  snprintf(output, 6, "%02u:%02u", hour % 24, minute % 60);
}

} // namespace WatchyUi