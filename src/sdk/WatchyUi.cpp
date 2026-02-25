#include "WatchyUi.h"
#include "WatchyPowerDiagnostics.h"
#include <Preferences.h>
#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <pgmspace.h>
#include <string.h>

namespace WatchyUi {

constexpr int16_t Theme::screenMargin;
constexpr int16_t Theme::titleBaseline;
constexpr int16_t Theme::contentBaseline;
constexpr int16_t Theme::listFirstBaseline;
constexpr int16_t Theme::footerBaseline;
constexpr int16_t Theme::listRowHeight;
constexpr uint8_t Theme::listVisibleRows;
constexpr uint32_t Screen::liveViewRefreshIntervalMs;

namespace {
uint8_t previousButtons = 0;
uint8_t auxiliaryWakePin = UINT8_MAX;
uint8_t auxiliaryWakeLevel = LOW;
WatchyUi::Input::WakeHandler auxiliaryWakeHandler = nullptr;
bool delayedBackPending = false;
uint32_t delayedBackAt = 0;
portMUX_TYPE delayedBackMux = portMUX_INITIALIZER_UNLOCKED;
#ifdef ARDUINO_ESP32S3_DEV
TaskHandle_t inputTask = nullptr;
#endif
RTC_DATA_ATTR bool themeLoaded = false;
RTC_DATA_ATTR uint8_t partialRefreshesSinceFull = 0;
RTC_DATA_ATTR uint32_t partialRefreshDebtPixels = 0;
Bounds dirtyRegions[Screen::maximumDirtyRegions]{};
uint8_t dirtyRegionCount = 0;

constexpr uint32_t buttonDebounceMs = 15;
constexpr uint32_t buttonReleaseTimeoutMs = 750;
constexpr uint32_t delayedBackTimeoutMs = 5000;
constexpr Bounds messageIconBounds{80, 58, 41, 41};
constexpr int16_t messageBodyTop =
  messageIconBounds.y + messageIconBounds.height + 7;
constexpr uint32_t fullScreenPixels = DISPLAY_WIDTH * DISPLAY_HEIGHT;
constexpr uint32_t displayBusyWaitSliceMs = 50;
constexpr uint32_t maximumPartialRefreshDebtPixels =
    59UL * fullScreenPixels;
constexpr uint8_t maximumConsecutivePartialRefreshes = 240;

constexpr uint8_t bayer16[16 * 16] PROGMEM = {
      0, 128,  32, 160,   8, 136,  40, 168,   2, 130,  34, 162,  10, 138,  42, 170,
    192,  64, 224,  96, 200,  72, 232, 104, 194,  66, 226,  98, 202,  74, 234, 106,
     48, 176,  16, 144,  56, 184,  24, 152,  50, 178,  18, 146,  58, 186,  26, 154,
    240, 112, 208,  80, 248, 120, 216,  88, 242, 114, 210,  82, 250, 122, 218,  90,
     12, 140,  44, 172,   4, 132,  36, 164,  14, 142,  46, 174,   6, 134,  38, 166,
    204,  76, 236, 108, 196,  68, 228, 100, 206,  78, 238, 110, 198,  70, 230, 102,
     60, 188,  28, 156,  52, 180,  20, 148,  62, 190,  30, 158,  54, 182,  22, 150,
    252, 124, 220,  92, 244, 116, 212,  84, 254, 126, 222,  94, 246, 118, 214,  86,
      3, 131,  35, 163,  11, 139,  43, 171,   1, 129,  33, 161,   9, 137,  41, 169,
    195,  67, 227,  99, 203,  75, 235, 107, 193,  65, 225,  97, 201,  73, 233, 105,
     51, 179,  19, 147,  59, 187,  27, 155,  49, 177,  17, 145,  57, 185,  25, 153,
    243, 115, 211,  83, 251, 123, 219,  91, 241, 113, 209,  81, 249, 121, 217,  89,
     15, 143,  47, 175,   7, 135,  39, 167,  13, 141,  45, 173,   5, 133,  37, 165,
    207,  79, 239, 111, 199,  71, 231, 103, 205,  77, 237, 109, 197,  69, 229, 101,
     63, 191,  31, 159,  55, 183,  23, 151,  61, 189,  29, 157,  53, 181,  21, 149,
    255, 127, 223,  95, 247, 119, 215,  87, 253, 125, 221,  93, 245, 117, 213,  85};

static_assert(sizeof(bayer16) == 256,
              "Bayer matrix must contain every 8-bit threshold");

bool fullRefreshDue(bool allowPeriodicFullRefresh) {
  return Watchy::display.epd2.initialFullRefreshPending() ||
         (allowPeriodicFullRefresh &&
          (partialRefreshDebtPixels >= maximumPartialRefreshDebtPixels ||
           partialRefreshesSinceFull >= maximumConsecutivePartialRefreshes));
}

void recordPartialRefresh(uint32_t pixels) {
  partialRefreshesSinceFull++;
  partialRefreshDebtPixels += pixels;
  WatchyDiagnostics::recordDirtyRefresh(pixels);
}

void resetPartialRefreshDebt() {
  partialRefreshesSinceFull = 0;
  partialRefreshDebtPixels = 0;
}

bool normalizeDirtyBounds(const Bounds &input, Bounds &output) {
  int16_t left = max<int16_t>(0, input.x);
  int16_t top = max<int16_t>(0, input.y);
  int16_t right = min<int16_t>(DISPLAY_WIDTH, input.x + input.width);
  int16_t bottom = min<int16_t>(DISPLAY_HEIGHT, input.y + input.height);
  if (left >= right || top >= bottom) return false;
  left &= ~7;
  right = min<int16_t>(DISPLAY_WIDTH, (right + 7) & ~7);
  output = {left, top, static_cast<int16_t>(right - left),
            static_cast<int16_t>(bottom - top)};
  return true;
}

Bounds unionBounds(const Bounds &first, const Bounds &second) {
  int16_t left = min(first.x, second.x);
  int16_t top = min(first.y, second.y);
  int16_t right = max(first.x + first.width,
                      second.x + second.width);
  int16_t bottom = max(first.y + first.height,
                       second.y + second.height);
  return {left, top, static_cast<int16_t>(right - left),
          static_cast<int16_t>(bottom - top)};
}

uint32_t boundsArea(const Bounds &bounds) {
  return static_cast<uint32_t>(bounds.width) * bounds.height;
}

bool boundsShouldMerge(const Bounds &first, const Bounds &second) {
  constexpr int16_t mergeGap = 4;
  bool near = first.x <= second.x + second.width + mergeGap &&
              second.x <= first.x + first.width + mergeGap &&
              first.y <= second.y + second.height + mergeGap &&
              second.y <= first.y + first.height + mergeGap;
  Bounds combined = unionBounds(first, second);
  constexpr uint32_t transactionAreaPenalty = 512;
  bool lowAreaPenalty = boundsArea(combined) <=
                        boundsArea(first) + boundsArea(second) +
                            transactionAreaPenalty;
  return near || lowAreaPenalty;
}

void clearDirtyRegions() {
  dirtyRegionCount = 0;
}

WakeupReason rememberDelayedBack() {
  portENTER_CRITICAL(&delayedBackMux);
  delayedBackPending = true;
  delayedBackAt = millis();
  portEXIT_CRITICAL(&delayedBackMux);
  return WakeupReason::BACK_PRESSED;
}

WakeupReason consumeDelayedBack() {
  portENTER_CRITICAL(&delayedBackMux);
  bool pending = delayedBackPending &&
                 millis() - delayedBackAt <= delayedBackTimeoutMs;
  delayedBackPending = false;
  delayedBackAt = 0;
  portEXIT_CRITICAL(&delayedBackMux);
  return pending ? WakeupReason::BACK_PRESSED : WakeupReason::NO_DELAY;
}

const gpio_num_t buttonPins[] = {
    static_cast<gpio_num_t>(MENU_BTN_PIN),
    static_cast<gpio_num_t>(BACK_BTN_PIN),
    static_cast<gpio_num_t>(UP_BTN_PIN),
    static_cast<gpio_num_t>(DOWN_BTN_PIN)};

bool enableButtonWakeup() {
  for (gpio_num_t pin : buttonPins) gpio_wakeup_enable(pin, GPIO_INTR_ANYEDGE);
  if (auxiliaryWakeHandler != nullptr) {
    gpio_wakeup_enable(static_cast<gpio_num_t>(auxiliaryWakePin), GPIO_INTR_ANYEDGE);
  }
  gpio_wakeup_enable(static_cast<gpio_num_t>(USB_DET_PIN), GPIO_INTR_ANYEDGE);
  return esp_sleep_enable_gpio_wakeup() == ESP_OK;
}

void disableButtonWakeup() {
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
  for (gpio_num_t pin : buttonPins) gpio_wakeup_disable(pin);
  if (auxiliaryWakeHandler != nullptr) {
    gpio_wakeup_disable(static_cast<gpio_num_t>(auxiliaryWakePin));
  }
  gpio_wakeup_disable(static_cast<gpio_num_t>(USB_DET_PIN));
}

void IRAM_ATTR notifyTaskFromIsr(void *argument) {
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(static_cast<TaskHandle_t>(argument),
                         &higherPriorityTaskWoken);
  if (higherPriorityTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

struct BackNotificationContext {
  TaskHandle_t task;
  volatile bool triggered;
};

void IRAM_ATTR notifyBackFromIsr(void *argument) {
  BackNotificationContext *context =
      static_cast<BackNotificationContext *>(argument);
  context->triggered = true;
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(context->task, &higherPriorityTaskWoken);
  if (higherPriorityTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

void detachButtonInterrupts() {
  for (gpio_num_t pin : buttonPins) detachInterrupt(pin);
}

bool waitForButtonNotification(uint32_t durationMs) {
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  for (gpio_num_t pin : buttonPins) {
    attachInterruptArg(pin, notifyTaskFromIsr, task, CHANGE);
  }
  TickType_t ticks = durationMs == UINT32_MAX
                         ? portMAX_DELAY
                         : max<TickType_t>(1, pdMS_TO_TICKS(durationMs));
  bool notified = ulTaskNotifyTake(pdTRUE, ticks) > 0;
  detachButtonInterrupts();
  return notified;
}

bool enableBackWakeup() {
  gpio_num_t pin = static_cast<gpio_num_t>(BACK_BTN_PIN);
  gpio_int_type_t trigger = digitalRead(BACK_BTN_PIN) == ACTIVE_LOW
                                ? GPIO_INTR_HIGH_LEVEL
                                : GPIO_INTR_LOW_LEVEL;
  if (gpio_wakeup_enable(pin, trigger) != ESP_OK) return false;
  if (esp_sleep_enable_gpio_wakeup() == ESP_OK) return true;
  gpio_wakeup_disable(pin);
  return false;
}

void disableBackWakeup() {
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
  gpio_wakeup_disable(static_cast<gpio_num_t>(BACK_BTN_PIN));
}

bool waitForBackNotification(uint32_t durationMs) {
  BackNotificationContext context{xTaskGetCurrentTaskHandle(), false};
  attachInterruptArg(BACK_BTN_PIN, notifyBackFromIsr, &context, FALLING);
  if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
    context.triggered = true;
  }
  uint32_t startedAt = millis();
  while (!context.triggered) {
    uint32_t elapsed = millis() - startedAt;
    if (elapsed >= durationMs) break;
    TickType_t ticks = max<TickType_t>(
        1, pdMS_TO_TICKS(durationMs - elapsed));
    if (ulTaskNotifyTake(pdTRUE, ticks) == 0) break;
  }
  detachInterrupt(BACK_BTN_PIN);
  return context.triggered;
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

template <typename LineVisitor>
uint16_t visitWrappedLines(const char *text, uint8_t columns,
                           LineVisitor visitor) {
  constexpr uint8_t maximumColumns = 31;
  columns = constrain(columns, static_cast<uint8_t>(1), maximumColumns);
  uint16_t lineNumber = 0;
  char line[maximumColumns + 1] = {};
  uint8_t lineLength = 0;

  auto emitLine = [&]() {
    line[lineLength] = '\0';
    visitor(lineNumber, line);
    lineNumber++;
    lineLength = 0;
    line[0] = '\0';
  };

  if (text == nullptr || text[0] == '\0') text = "(empty)";

  const char *cursor = text;
  while (*cursor != '\0') {
    if (*cursor == '\r') {
      cursor++;
      continue;
    }
    if (*cursor == '\n') {
      emitLine();
      cursor++;
      continue;
    }
    if (isspace(static_cast<unsigned char>(*cursor))) {
      cursor++;
      continue;
    }

    const char *word = cursor;
    size_t wordLength = 0;
    while (word[wordLength] != '\0' && word[wordLength] != '\r' &&
           word[wordLength] != '\n' &&
           !isspace(static_cast<unsigned char>(word[wordLength]))) {
      wordLength++;
    }
    cursor += wordLength;

    size_t wordOffset = 0;
    while (wordOffset < wordLength) {
      if (lineLength >= columns) {
        emitLine();
        continue;
      }
      uint8_t separator = lineLength == 0 ? 0 : 1;
      uint8_t used = lineLength + separator;
      uint8_t room = used < columns ? columns - used : 0;
      size_t remaining = wordLength - wordOffset;
      if (remaining <= room) {
        if (separator != 0) line[lineLength++] = ' ';
        while (wordOffset < wordLength) {
          line[lineLength++] = word[wordOffset++];
        }
      } else if (lineLength > 0) {
        emitLine();
      } else {
        while (room > 0 && wordOffset < wordLength) {
          line[lineLength++] = word[wordOffset++];
          room--;
        }
        emitLine();
      }
    }
  }
  if (lineLength > 0) emitLine();
  return lineNumber;
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

bool GrayPaint::pixelIsWhite(int16_t x, int16_t y, Gray8 gray) {
  if (gray == GRAY_BLACK) return false;
  if (gray == GRAY_WHITE) return true;
  uint8_t threshold = pgm_read_byte(
      &bayer16[(static_cast<uint16_t>(y) & 15U) * 16U +
               (static_cast<uint16_t>(x) & 15U)]);
  return threshold < gray;
}

void GrayPaint::pixel(int16_t x, int16_t y, Gray8 gray) {
  if (x < 0 || y < 0 || x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
  Watchy::display.drawPixel(x, y, pixelIsWhite(x, y, gray)
                                      ? GxEPD_WHITE
                                      : GxEPD_BLACK);
}

void GrayPaint::fillRect(const Bounds &bounds, Gray8 gray) {
  int16_t left = max<int16_t>(0, bounds.x);
  int16_t top = max<int16_t>(0, bounds.y);
  int16_t right = min<int16_t>(DISPLAY_WIDTH, bounds.x + bounds.width);
  int16_t bottom = min<int16_t>(DISPLAY_HEIGHT, bounds.y + bounds.height);
  if (left >= right || top >= bottom) return;
  if (gray == GRAY_BLACK || gray == GRAY_WHITE) {
    Watchy::display.fillRect(left, top, right - left, bottom - top,
                            gray == GRAY_WHITE ? GxEPD_WHITE
                                               : GxEPD_BLACK);
    return;
  }
  for (int16_t y = top; y < bottom; y++) {
    for (int16_t x = left; x < right; x++) pixel(x, y, gray);
  }
}

void GrayPaint::fillRoundRect(const Bounds &bounds, int16_t radius,
                              Gray8 gray) {
  if (bounds.width <= 0 || bounds.height <= 0) return;
  radius = constrain(radius, 0, min(bounds.width, bounds.height) / 2);
  for (int16_t localY = 0; localY < bounds.height; localY++) {
    for (int16_t localX = 0; localX < bounds.width; localX++) {
      int16_t cornerX = localX < radius
                            ? radius - 1
                            : (localX >= bounds.width - radius
                                   ? bounds.width - radius
                                   : localX);
      int16_t cornerY = localY < radius
                            ? radius - 1
                            : (localY >= bounds.height - radius
                                   ? bounds.height - radius
                                   : localY);
      int16_t dx = localX - cornerX;
      int16_t dy = localY - cornerY;
      if (dx * dx + dy * dy <= radius * radius) {
        pixel(bounds.x + localX, bounds.y + localY, gray);
      }
    }
  }
}

void GrayPaint::line(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     Gray8 gray) {
  int16_t dx = abs(x1 - x0);
  int16_t stepX = x0 < x1 ? 1 : -1;
  int16_t dy = -abs(y1 - y0);
  int16_t stepY = y0 < y1 ? 1 : -1;
  int16_t error = dx + dy;
  while (true) {
    pixel(x0, y0, gray);
    if (x0 == x1 && y0 == y1) break;
    int16_t doubledError = error * 2;
    if (doubledError >= dy) {
      error += dy;
      x0 += stepX;
    }
    if (doubledError <= dx) {
      error += dx;
      y0 += stepY;
    }
  }
}

void GrayPaint::bitmap8(int16_t x, int16_t y, uint16_t width,
                        uint16_t height, const uint8_t *pixels,
                        bool programMemory) {
  if (pixels == nullptr || width == 0 || height == 0) return;
  int32_t sourceLeft = max<int32_t>(0, -static_cast<int32_t>(x));
  int32_t sourceTop = max<int32_t>(0, -static_cast<int32_t>(y));
  int32_t sourceRight = min<int32_t>(
      width, static_cast<int32_t>(DISPLAY_WIDTH) - x);
  int32_t sourceBottom = min<int32_t>(
      height, static_cast<int32_t>(DISPLAY_HEIGHT) - y);
  if (sourceLeft >= sourceRight || sourceTop >= sourceBottom) return;
  for (int32_t row = sourceTop; row < sourceBottom; row++) {
    for (int32_t column = sourceLeft; column < sourceRight; column++) {
      size_t index = static_cast<size_t>(row) * width + column;
      Gray8 gray = programMemory ? pgm_read_byte(pixels + index)
                                 : pixels[index];
      pixel(static_cast<int16_t>(x + column),
        static_cast<int16_t>(y + row), gray);
    }
  }
}

void GrayPaint::gradient(const Bounds &bounds, Gray8 start, Gray8 end,
                         bool horizontal) {
  int16_t extent = horizontal ? bounds.width : bounds.height;
  if (extent <= 0) return;
  int16_t denominator = max<int16_t>(1, extent - 1);
  for (int16_t localY = 0; localY < bounds.height; localY++) {
    for (int16_t localX = 0; localX < bounds.width; localX++) {
      int16_t position = horizontal ? localX : localY;
      int32_t value = static_cast<int32_t>(start) +
          (static_cast<int32_t>(end) - start) * position / denominator;
      pixel(bounds.x + localX, bounds.y + localY,
            static_cast<Gray8>(value));
    }
  }
}

uint16_t Theme::background() {
  return DARKMODE ? GxEPD_BLACK : GxEPD_WHITE;
}

uint16_t Theme::foreground() {
  return DARKMODE ? GxEPD_WHITE : GxEPD_BLACK;
}

Gray8 Theme::tone(ToneRole role) {
  static constexpr Gray8 light[] = {
      255, 232, 208, 0, 64, 128, 176, 144, 48, 216};
  static constexpr Gray8 dark[] = {
      0, 24, 48, 255, 192, 128, 80, 96, 208, 40};
  uint8_t index = static_cast<uint8_t>(role);
  if (index >= sizeof(light) / sizeof(light[0])) {
    return DARKMODE ? GRAY_WHITE : GRAY_BLACK;
  }
  return DARKMODE ? dark[index] : light[index];
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
  clearDirtyRegions();
  Theme::load();
  Watchy::ensureDisplayInitialized();
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

void Screen::invalidate(const Bounds &bounds) {
  Bounds normalized{};
  if (!normalizeDirtyBounds(bounds, normalized)) return;

  for (uint8_t index = 0; index < dirtyRegionCount;) {
    if (!boundsShouldMerge(dirtyRegions[index], normalized)) {
      index++;
      continue;
    }
    normalized = unionBounds(dirtyRegions[index], normalized);
    dirtyRegions[index] = dirtyRegions[--dirtyRegionCount];
    index = 0;
  }

  if (dirtyRegionCount < maximumDirtyRegions) {
    dirtyRegions[dirtyRegionCount++] = normalized;
    return;
  }

  uint8_t best = 0;
  uint32_t bestPenalty = UINT32_MAX;
  for (uint8_t index = 0; index < dirtyRegionCount; index++) {
    Bounds combined = unionBounds(dirtyRegions[index], normalized);
    uint32_t penalty = boundsArea(combined) -
                       boundsArea(dirtyRegions[index]);
    if (penalty < bestPenalty) {
      best = index;
      bestPenalty = penalty;
    }
  }
  dirtyRegions[best] = unionBounds(dirtyRegions[best], normalized);
}

void Screen::presentDirty(int nextGuiState) {
  if (dirtyRegionCount == 0) {
    guiState = nextGuiState;
    return;
  }
#ifdef WATCHY_DETERMINISTIC_GALLERY
  Watchy::display.setFullWindow();
    Watchy::display.display(true);
  clearDirtyRegions();
  guiState = nextGuiState;
  return;
#endif
  for (uint8_t index = 0; index < dirtyRegionCount; index++) {
    const Bounds &bounds = dirtyRegions[index];
    Watchy::display.setPartialWindow(bounds.x, bounds.y,
                                    bounds.width, bounds.height);
    Watchy::display.display(true);
    recordPartialRefresh(boundsArea(bounds));
  }
  Watchy::display.setFullWindow();
  clearDirtyRegions();
  guiState = nextGuiState;
}

void Screen::present(int nextGuiState, bool allowPeriodicFullRefresh) {
  bool fullRefresh = fullRefreshDue(allowPeriodicFullRefresh);
  Watchy::display.display(!fullRefresh);
  if (fullRefresh) {
    resetPartialRefreshDebt();
  } else {
    recordPartialRefresh(fullScreenPixels);
  }
  clearDirtyRegions();
  guiState = nextGuiState;
}

void Screen::present(const Bounds &dirtyBounds, int nextGuiState) {
  clearDirtyRegions();
  invalidate(dirtyBounds);
  presentDirty(nextGuiState);
}

void Widget::bodyText(int16_t x, int16_t y) {
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(Theme::foreground());
  Watchy::display.setCursor(x, y);
}

void Widget::separator(int16_t y) {
  GrayPaint::line(Theme::screenMargin, y,
                  DISPLAY_WIDTH - Theme::screenMargin - 1, y,
                  Theme::tone(ToneRole::Separator));
}

void Widget::footer(const char *commands) {
  if (commands == nullptr || commands[0] == '\0') {
    return;
  }
  GrayPaint::line(Theme::screenMargin, 178,
                  DISPLAY_WIDTH - Theme::screenMargin - 1, 178,
                  Theme::tone(ToneRole::Separator));
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
  GrayPaint::fillRoundRect(
      Bounds{Theme::screenMargin, y,
             DISPLAY_WIDTH - Theme::screenMargin * 2, 17},
      3, Theme::tone(ToneRole::SurfaceRaised));
  Watchy::display.fillRect(Theme::screenMargin + 3, y + 3,
                           static_cast<int16_t>(
                               (DISPLAY_WIDTH - Theme::screenMargin * 2 - 6) *
                               value),
                           11, color);
}

void Widget::checkbox(const char *label, bool checked, int16_t y,
                      bool focused, bool enabled) {
  const uint16_t foreground = Theme::foreground();
  if (focused || !enabled) {
    GrayPaint::fillRoundRect(
        Bounds{Theme::screenMargin, static_cast<int16_t>(y - 6),
               DISPLAY_WIDTH - Theme::screenMargin * 2, 20},
        3, Theme::tone(enabled ? ToneRole::Selection
                               : ToneRole::Disabled));
  }
  uint16_t color = foreground;
  const Bounds controlBounds{8, static_cast<int16_t>(y - 2), 12, 12};
  Canvas::outline(controlBounds, color);
  if (checked) {
    Bounds markBounds = controlBounds.inset(3);
    Watchy::display.fillRect(markBounds.x, markBounds.y, markBounds.width,
                             markBounds.height, color);
  }
  bodyText(28, y);
  Watchy::display.setTextColor(color);
  Watchy::display.print(label);
}

void Widget::radio(const char *label, bool selected, int16_t y,
                   bool focused, bool enabled) {
  const uint16_t foreground = Theme::foreground();
  if (focused || !enabled) {
    GrayPaint::fillRoundRect(
        Bounds{Theme::screenMargin, static_cast<int16_t>(y - 6),
               DISPLAY_WIDTH - Theme::screenMargin * 2, 20},
        3, Theme::tone(enabled ? ToneRole::Selection
                               : ToneRole::Disabled));
  }
  uint16_t color = foreground;
  const Bounds controlBounds{8, static_cast<int16_t>(y - 2), 13, 13};
  Canvas::circle(controlBounds, color);
  if (selected) Canvas::circle(controlBounds.inset(3), color, true);
  bodyText(28, y);
  Watchy::display.setTextColor(color);
  Watchy::display.print(label);
}

void Widget::toggle(bool enabled, int16_t x, int16_t y, bool focused) {
  const uint16_t foreground = Theme::foreground();
  uint16_t color = foreground;
  if (focused) {
    GrayPaint::fillRoundRect(Bounds{static_cast<int16_t>(x - 3),
                                    static_cast<int16_t>(y - 3), 36, 22},
                             4, Theme::tone(ToneRole::Selection));
  }
  Watchy::display.drawRoundRect(x, y, 30, 16, 8, color);
  Watchy::display.fillCircle(enabled ? x + 22 : x + 8, y + 8, 5, color);
}

void ValueView::draw(const ValueModel &model) {
  Screen::begin(model.title);
  Widget::separator();
  constexpr Bounds valueBounds{8, 34, 184, 70};
  constexpr Bounds statusBounds{30, 111, 140, 20};
  GrayPaint::fillRoundRect(valueBounds, 4, Theme::tone(ToneRole::Surface));
  Watchy::display.setFont();
  uint8_t valueSize = strlen(model.value) > 8 ? 2 :
                      strlen(model.value) > 5 ? 3 : 4;
  Canvas::centeredText(valueBounds.inset(4), model.value, valueSize,
                       Theme::foreground());
  if (model.status != nullptr && model.status[0] != '\0') {
    bool urgent = strstr(model.status, "FINISHED") != nullptr ||
                  strstr(model.status, "EXPIRED") != nullptr ||
                  strstr(model.status, "ALERT") != nullptr;
    bool active = strstr(model.status, "RUNNING") != nullptr ||
                  strstr(model.status, "FOCUS") != nullptr ||
                  strstr(model.status, "WORK") != nullptr ||
                  strstr(model.status, "ARMED") != nullptr ||
                  strstr(model.status, "ON") != nullptr;
    GrayPaint::fillRoundRect(statusBounds, 3,
                             Theme::tone(urgent ? ToneRole::Selection
                                                 : active ? ToneRole::SurfaceRaised
                                                          : ToneRole::Surface));
    uint8_t statusSize = strlen(model.status) > 17 ? 1 : 2;
    Watchy::display.setFont();
    Watchy::display.setTextSize(statusSize);
    int16_t textX;
    int16_t textY;
    uint16_t textWidth;
    uint16_t textHeight;
    Watchy::display.getTextBounds(model.status, 0, 0, &textX, &textY,
                                  &textWidth, &textHeight);
    constexpr int16_t iconSlotWidth = 12;
    constexpr int16_t iconTextGap = 6;
    int16_t groupWidth = static_cast<int16_t>(iconSlotWidth + iconTextGap +
                                              textWidth);
    int16_t groupLeft = static_cast<int16_t>(
        statusBounds.x + (statusBounds.width - groupWidth) / 2);
    int16_t iconCenterX =
        static_cast<int16_t>(groupLeft + iconSlotWidth / 2);
    int16_t iconCenterY =
        static_cast<int16_t>(statusBounds.y + statusBounds.height / 2);
    if (urgent) {
      Watchy::display.drawTriangle(
          iconCenterX, iconCenterY - 6, iconCenterX - 6, iconCenterY + 5,
          iconCenterX + 6, iconCenterY + 5, Theme::foreground());
    } else if (active) {
      Watchy::display.fillCircle(iconCenterX, iconCenterY, 4,
                                 Theme::foreground());
    } else {
      Watchy::display.drawCircle(iconCenterX, iconCenterY, 4,
                                 Theme::foreground());
    }
    int16_t textLeft =
        static_cast<int16_t>(groupLeft + iconSlotWidth + iconTextGap);
    Canvas::centeredText(
        {textLeft, statusBounds.y, static_cast<int16_t>(textWidth),
         statusBounds.height},
        model.status, statusSize, Theme::foreground());
  }
  bool hasProgress = model.progress >= 0.0f && model.progress <= 1.0f;
  if (hasProgress) {
    Widget::progress(model.progress, 137);
  }
  if (model.detail != nullptr && model.detail[0] != '\0') {
    Widget::paragraph(model.detail, Theme::screenMargin,
                      hasProgress ? 157 : 143, 32,
                      hasProgress ? 2 : 3);
  }
  Widget::footer(model.footer);
}

void Feedback::showMessage(const char *title, const char *message,
                           MessageKind kind, const char *footer) {
  Screen::begin(title);
  Widget::separator();
  GrayPaint::fillRoundRect(Bounds{2, 28, 196, 144}, 4,
                           Theme::tone(ToneRole::Surface));
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
    if (event == Event::MENU) return true;
    if (event == Event::BACK) return false;
  }
}

void Feedback::toast(const char *message) {
  const uint16_t foreground = Theme::foreground();
  constexpr Bounds toastBounds{4, 158, 192, 38};
  GrayPaint::fillRoundRect(toastBounds, 4,
                           Theme::tone(ToneRole::SurfaceRaised));
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(foreground);
  Watchy::display.setCursor(10, 181);
  Watchy::display.print(message);
  Screen::present(toastBounds, guiState);
}

uint16_t ScrollableTextView::lineCount(const char *text, uint8_t columns) {
  return visitWrappedLines(text, columns,
                           [](uint16_t, const char *) {});
}

uint16_t ScrollableTextView::maximumFirstLine(uint16_t totalLines,
                                              uint8_t visibleLines) {
  visibleLines = max<uint8_t>(1, visibleLines);
  return totalLines > visibleLines ? totalLines - visibleLines : 0;
}

uint16_t ScrollableTextView::previous(uint16_t firstLine,
                                      uint8_t visibleLines) {
  uint16_t step = max<uint8_t>(1, visibleLines - 1);
  return firstLine > step ? firstLine - step : 0;
}

uint16_t ScrollableTextView::next(uint16_t firstLine, uint16_t totalLines,
                                  uint8_t visibleLines) {
  uint16_t maximum = maximumFirstLine(totalLines, visibleLines);
  uint16_t step = max<uint8_t>(1, visibleLines - 1);
  return min<uint16_t>(maximum, firstLine + step);
}

void ScrollableTextView::draw(const ScrollableTextModel &model) {
  uint8_t columns = max<uint8_t>(1, model.columns);
  uint8_t visibleLines = max<uint8_t>(1, model.visibleLines);
  uint8_t lineHeight = max<uint8_t>(1, model.lineHeight);
  uint16_t totalLines = lineCount(model.text, columns);
  uint16_t firstLine = min<uint16_t>(
      model.firstLine, maximumFirstLine(totalLines, visibleLines));

  Screen::begin(model.title);
  Widget::separator();
  Widget::bodyText(Theme::screenMargin, 33);
  visitWrappedLines(
      model.text, columns,
      [&](uint16_t lineNumber, const char *line) {
        if (lineNumber < firstLine ||
            lineNumber >= firstLine + visibleLines) {
          return;
        }
        uint16_t row = lineNumber - firstLine;
        Watchy::display.setCursor(
            Theme::screenMargin,
            33 + static_cast<int16_t>(row) * lineHeight);
        Watchy::display.print(line);
      });

  if (totalLines > visibleLines) {
    constexpr int16_t trackTop = 27;
    constexpr int16_t trackHeight = 145;
    uint16_t maximum = maximumFirstLine(totalLines, visibleLines);
    int16_t thumbHeight = max<int16_t>(
        8, static_cast<int16_t>(trackHeight) * visibleLines / totalLines);
    int16_t thumbTravel = trackHeight - thumbHeight;
    int16_t thumbTop = trackTop +
        (maximum == 0 ? 0 :
         static_cast<int32_t>(thumbTravel) * firstLine / maximum);
    Watchy::display.drawFastVLine(DISPLAY_WIDTH - 2, trackTop, trackHeight,
                                  Theme::foreground());
    Watchy::display.fillRect(DISPLAY_WIDTH - 4, thumbTop, 4, thumbHeight,
                             Theme::foreground());
  }
  Widget::footer(model.footer);
}

void ScrollableTextView::show(const char *title, const char *text,
                              const char *footer, uint8_t columns,
                              uint8_t visibleLines, uint8_t lineHeight) {
  uint16_t totalLines = lineCount(text, columns);
  uint16_t firstLine = 0;
  Input::begin();
  while (true) {
    draw(ScrollableTextModel{title, text, footer, firstLine, columns,
                             visibleLines, lineHeight});
    Screen::present();
    Event event = Input::wait();
    if (event == Event::BACK) return;
    uint16_t nextLine = firstLine;
    if (event == Event::UP) {
      nextLine = previous(firstLine, visibleLines);
    } else if (event == Event::DOWN) {
      nextLine = next(firstLine, totalLines, visibleLines);
    }
    if (nextLine != firstLine) firstLine = nextLine;
  }
}

void Input::begin() {
  inputTask = xTaskGetCurrentTaskHandle();
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);
  previousButtons = 0;
}

void Input::setAuxiliaryWakeSource(uint8_t pin, uint8_t activeLevel,
                                   WakeHandler handler) {
  auxiliaryWakePin = pin;
  auxiliaryWakeLevel = activeLevel;
  auxiliaryWakeHandler = handler;
}

void Power::idle(uint32_t durationMs) {
#ifdef ARDUINO_ESP32S3_DEV
  if (usbPluggedIn()) {
    constexpr uint32_t usbPollIntervalMs = 100;
    uint32_t waitDurationMs = durationMs == UINT32_MAX
                                  ? usbPollIntervalMs
                                  : durationMs < usbPollIntervalMs
                                        ? durationMs
                                        : usbPollIntervalMs;
    waitForButtonNotification(waitDurationMs);
    return;
  }
  if (WiFi.getMode() == WIFI_OFF && !btStarted() && enableButtonWakeup()) {
    bool timerEnabled = durationMs != UINT32_MAX;
    if (!timerEnabled ||
        esp_sleep_enable_timer_wakeup(
            static_cast<uint64_t>(durationMs) * 1000ULL) == ESP_OK) {
      uint32_t sleepStartedAt = millis();
      esp_err_t result = esp_light_sleep_start();
      WatchyDiagnostics::recordLightSleep(millis() - sleepStartedAt);
      if (timerEnabled) {
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      }
      disableButtonWakeup();
      if (result == ESP_OK) return;
    } else {
      disableButtonWakeup();
    }
  }
  waitForButtonNotification(durationMs);
  return;
#endif
  TickType_t ticks = pdMS_TO_TICKS(
      durationMs == UINT32_MAX || durationMs > 10 ? 10 : durationMs);
  vTaskDelay(ticks == 0 ? 1 : ticks);
}

bool Power::usbPluggedIn() {
#ifdef ARDUINO_ESP32S3_DEV
  rtc_gpio_deinit(static_cast<gpio_num_t>(USB_DET_PIN));
  pinMode(USB_DET_PIN, INPUT);
  USB_PLUGGED_IN = digitalRead(USB_DET_PIN) == HIGH;
  return USB_PLUGGED_IN;
#else
  return false;
#endif
}

void Power::waitForDisplayReady() {
  (void)waitForDisplayReady(3000);
}

bool Power::waitForDisplayReady(uint32_t timeoutMs) {
  uint32_t startedAt = millis();
  while (digitalRead(DISPLAY_BUSY) != LOW) {
    if (timeoutMs != UINT32_MAX) {
      uint32_t elapsed = millis() - startedAt;
      if (elapsed >= timeoutMs) return false;
      uint32_t remaining = timeoutMs - elapsed;
      uint32_t delayMs = min<uint32_t>(remaining, displayBusyWaitSliceMs);
      vTaskDelay(pdMS_TO_TICKS(delayMs == 0 ? 1 : delayMs));
    } else {
      vTaskDelay(pdMS_TO_TICKS(displayBusyWaitSliceMs));
    }
  }
  return true;
}

WakeupReason deepSleepDelay(uint32_t durationMs) {
  if (durationMs == 0) return WakeupReason::NO_DELAY;
#ifdef ARDUINO_ESP32S3_DEV
  if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
    return rememberDelayedBack();
  }
  if (!Power::usbPluggedIn() && WiFi.getMode() == WIFI_OFF &&
      !btStarted() && enableBackWakeup()) {
    if (esp_sleep_enable_timer_wakeup(
            static_cast<uint64_t>(durationMs) * 1000ULL) == ESP_OK) {
      uint32_t sleepStartedAt = millis();
      esp_err_t result = esp_light_sleep_start();
      esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();
      WatchyDiagnostics::recordLightSleep(millis() - sleepStartedAt);
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      disableBackWakeup();
      if (result == ESP_OK) {
        if (wakeCause == ESP_SLEEP_WAKEUP_GPIO ||
            digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
          return rememberDelayedBack();
        }
        return WakeupReason::DEEP_SLEEP_DELAY;
      }
    } else {
      disableBackWakeup();
    }
  }
  if (inputTask == nullptr || xTaskGetCurrentTaskHandle() != inputTask) {
    TickType_t ticks = pdMS_TO_TICKS(durationMs);
    vTaskDelay(ticks == 0 ? 1 : ticks);
    return WakeupReason::SCHEDULER_DELAY;
  }
  if (waitForBackNotification(durationMs)) {
    return rememberDelayedBack();
  }
  return WakeupReason::SCHEDULER_DELAY;
#else
  TickType_t ticks = pdMS_TO_TICKS(durationMs);
  vTaskDelay(ticks == 0 ? 1 : ticks);
  return WakeupReason::SCHEDULER_DELAY;
#endif
}

Event Input::poll() {
  if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
    return Event::BACK;
  }
  if (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW) {
    return Event::MENU;
  }
  if (digitalRead(UP_BTN_PIN) == ACTIVE_LOW) {
    return Event::UP;
  }
  if (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW) {
    return Event::DOWN;
  }
  return Event::NONE;
}

Event Input::wait(uint32_t timeoutMs) {
  uint32_t startedAt = millis();
  while (timeoutMs == UINT32_MAX || millis() - startedAt < timeoutMs) {
    if (auxiliaryWakeHandler != nullptr) auxiliaryWakeHandler();
    Event event = poll();
    if (event != Event::NONE) return event;
    uint32_t elapsed = millis() - startedAt;
    if (timeoutMs != UINT32_MAX && elapsed >= timeoutMs) break;
    uint32_t remaining = timeoutMs == UINT32_MAX
                             ? UINT32_MAX
                             : timeoutMs - elapsed;
    Power::idle(remaining);
  }
  return Event::NONE;
}

Event Input::waitScheduled(uint32_t timeoutMs) {
  uint32_t startedAt = millis();
  while (timeoutMs == UINT32_MAX || millis() - startedAt < timeoutMs) {
    if (auxiliaryWakeHandler != nullptr) auxiliaryWakeHandler();
    Event event = poll();
    if (event != Event::NONE) return event;
    uint32_t elapsed = millis() - startedAt;
    if (timeoutMs != UINT32_MAX && elapsed >= timeoutMs) break;
    uint32_t remaining = timeoutMs == UINT32_MAX
                             ? UINT32_MAX
                             : timeoutMs - elapsed;
    waitForButtonNotification(remaining);
  }
  return Event::NONE;
}

Event Input::waitNotified(uint32_t timeoutMs) {
  Event event = poll();
  if (event != Event::NONE) return event;
  return wait(timeoutMs);
}

void Input::waitForRelease(Event event) {
  while (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW && event == Event::BACK) {
    Power::idle(buttonDebounceMs);
  }
  while (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW && event == Event::MENU) {
    Power::idle(buttonDebounceMs);
  }
  while (digitalRead(UP_BTN_PIN) == ACTIVE_LOW && event == Event::UP) {
    Power::idle(buttonDebounceMs);
  }
  while (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW && event == Event::DOWN) {
    Power::idle(buttonDebounceMs);
  }
}

namespace {

uint8_t listVisibleRows(const ListModel &model) {
  return constrain(model.visibleRows, 1, Theme::listVisibleRows);
}

uint8_t listSelectedItem(const ListModel &model) {
  return model.itemCount == 0 ? 0 :
         model.selectedIndex % model.itemCount;
}

uint8_t listFirstItem(const ListModel &model, uint8_t selected) {
  uint8_t visibleRows = listVisibleRows(model);
  return selected >= visibleRows ? selected - visibleRows + 1 : 0;
}

void configureListRowFont(const ListModel &model) {
  if (model.compactText) {
    Watchy::display.setFont();
  } else {
    Watchy::display.setFont(&FreeMonoBold9pt7b);
  }
  Watchy::display.setTextSize(1);
}

Bounds listRowBounds(uint8_t item, uint8_t firstItem) {
  int16_t baseline = Theme::listFirstBaseline +
                     (item - firstItem) * Theme::listRowHeight;
  return {Theme::screenMargin, static_cast<int16_t>(baseline - 17),
          DISPLAY_WIDTH - Theme::screenMargin * 2,
          Theme::listRowHeight};
}

void drawListRow(const ListModel &model, uint8_t item, uint8_t firstItem,
                 uint8_t selected, uint16_t background,
                 uint16_t foreground) {
  int16_t baseline = Theme::listFirstBaseline +
                     (item - firstItem) * Theme::listRowHeight;
  bool isSelected = item == selected;
  if (isSelected) {
    Watchy::display.fillRect(Theme::screenMargin, baseline - 16,
                             DISPLAY_WIDTH - Theme::screenMargin * 2,
                             Theme::listRowHeight - 1, foreground);
  }
  Watchy::display.setTextColor(isSelected ? background : foreground);
  int16_t labelX = 5;
  if (model.activeIndex >= 0) {
    int16_t centerY = baseline - 6;
    const Bounds indicatorBounds{
        6, static_cast<int16_t>(centerY - 4), 9, 9};
    Canvas::circle(indicatorBounds,
                   isSelected ? background : foreground);
    if (item == model.activeIndex) {
      Canvas::circle(indicatorBounds.inset(2),
                     isSelected ? background : foreground, true);
    }
    labelX = 16;
  }
  int16_t textY = model.compactText ? baseline - 10 : baseline;
  Watchy::display.setCursor(labelX, textY);
  printClipped(model.labelAt(item, model.context),
               model.detailAt == nullptr ? 26 : 14);
  if (model.detailAt != nullptr) {
    Watchy::display.setCursor(104, textY);
    printClipped(model.detailAt(item, model.context),
                 model.showDisclosure ? 12 : 14);
  }
  if (model.showDisclosure) {
    Watchy::display.setCursor(183, textY);
    Watchy::display.print('>');
  }
}

void drawListBody(const ListModel &model, uint8_t selected,
                  uint8_t firstItem, uint16_t background,
                  uint16_t foreground) {
  uint8_t visibleRows = listVisibleRows(model);
  uint8_t lastItem = min<int>(firstItem + visibleRows, model.itemCount);
  configureListRowFont(model);
  for (uint8_t item = firstItem; item < lastItem; item++) {
    drawListRow(model, item, firstItem, selected, background, foreground);
  }
  Watchy::display.setTextColor(foreground);
  if (firstItem > 0) {
    Watchy::display.fillTriangle(193, 25, 188, 31, 198, 31, foreground);
  }
  if (lastItem < model.itemCount) {
    Watchy::display.fillTriangle(188, 169, 198, 169, 193, 175, foreground);
  }
}

} // namespace

void ListView::draw(const ListModel &model) {
  Screen::begin(model.title);
  const uint16_t background = Theme::background();
  const uint16_t foreground = Theme::foreground();
  Watchy::display.drawFastHLine(Theme::screenMargin, 23,
                                DISPLAY_WIDTH - Theme::screenMargin * 2,
                                foreground);

  uint8_t selected = listSelectedItem(model);
  uint8_t firstItem = listFirstItem(model, selected);
  drawListBody(model, selected, firstItem, background, foreground);

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
                  selectedIndex, visibleRows, activeIndex, false, false};
  draw(model);
}

void ListView::presentSelectionChange(const ListModel &model,
                                      uint8_t previousSelectedIndex,
                                      int nextGuiState) {
  if (model.itemCount == 0) return;
  const uint16_t background = Theme::background();
  const uint16_t foreground = Theme::foreground();
  uint8_t selected = listSelectedItem(model);
  uint8_t previous = previousSelectedIndex % model.itemCount;
  if (selected == previous) return;
  uint8_t firstItem = listFirstItem(model, selected);
  uint8_t previousFirstItem = listFirstItem(model, previous);
  if (firstItem != previousFirstItem) {
    constexpr Bounds bodyBounds{Theme::screenMargin, 24,
                                DISPLAY_WIDTH - Theme::screenMargin * 2,
                                152};
    Watchy::display.fillRect(bodyBounds.x, bodyBounds.y, bodyBounds.width,
                            bodyBounds.height, background);
    drawListBody(model, selected, firstItem, background, foreground);
    Screen::invalidate(bodyBounds);
  } else {
    Bounds previousBounds = listRowBounds(previous, firstItem);
    Bounds selectedBounds = listRowBounds(selected, firstItem);
    Watchy::display.fillRect(previousBounds.x, previousBounds.y,
                            previousBounds.width, previousBounds.height,
                            background);
    Watchy::display.fillRect(selectedBounds.x, selectedBounds.y,
                            selectedBounds.width, selectedBounds.height,
                            background);
    configureListRowFont(model);
    drawListRow(model, previous, firstItem, selected, background, foreground);
    drawListRow(model, selected, firstItem, selected, background, foreground);
    Screen::invalidate(previousBounds);
    Screen::invalidate(selectedBounds);
  }
  Screen::presentDirty(nextGuiState);
}

void ListView::presentSelectionChange(
    const char *title, const char *const labels[], uint8_t itemCount,
    uint8_t selectedIndex, uint8_t previousSelectedIndex,
    const char *footer, int16_t activeIndex, uint8_t visibleRows,
    int nextGuiState) {
  LabelArray context{labels};
  ListModel model{title, labelFromArray, nullptr, &context, footer, itemCount,
                  selectedIndex, visibleRows, activeIndex, false, false};
  presentSelectionChange(model, previousSelectedIndex, nextGuiState);
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