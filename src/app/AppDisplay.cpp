#include "AppDisplay.h"
#include "WatchyUi.h"
#include <math.h>
#include <string.h>

namespace AppVisual {
namespace {

constexpr int16_t kInset = 4;

float clamped(float value) {
  return constrain(value, 0.0f, 1.0f);
}

uint8_t textSizeForValue(const char *value) {
  size_t length = value == nullptr ? 0 : strlen(value);
  if (length <= 3) return 4;
  if (length <= 6) return 3;
  return 2;
}

void centered(const WatchyUi::Bounds &bounds, const char *text,
              uint8_t size = 1) {
  WatchyUi::Canvas::centeredText(bounds, text == nullptr ? "" : text, size,
                                 WatchyUi::Theme::foreground());
}

} // namespace

void drawStatusIcon(const WatchyUi::Bounds &bounds, StatusIcon icon,
                    bool emphasized) {
  const uint16_t foreground = WatchyUi::Theme::foreground();
  const uint16_t background = WatchyUi::Theme::background();
  const int16_t centerX = bounds.x + bounds.width / 2;
  const int16_t centerY = bounds.y + bounds.height / 2;
  const int16_t radius = max<int16_t>(5, min(bounds.width, bounds.height) / 2 - 2);

  if (emphasized) {
    WatchyUi::GrayPaint::fillRoundRect(
        bounds, min<int16_t>(6, radius),
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  }

  switch (icon) {
  case StatusIcon::WARNING:
    Watchy::display.drawTriangle(centerX, bounds.y + 2, bounds.x + 2,
                                 bounds.y + bounds.height - 3,
                                 bounds.x + bounds.width - 3,
                                 bounds.y + bounds.height - 3, foreground);
    centered({static_cast<int16_t>(centerX - 8),
              static_cast<int16_t>(centerY - 3), 16, 16}, "!", 2);
    break;
  case StatusIcon::RADIO:
    Watchy::display.fillCircle(centerX, centerY + radius / 2, 2, foreground);
    Watchy::display.drawLine(centerX, centerY + radius / 2, centerX,
                             centerY - radius / 2, foreground);
    Watchy::display.drawLine(centerX - radius / 2, centerY - radius / 4,
                             centerX - radius, centerY - radius / 2,
                             foreground);
    Watchy::display.drawLine(centerX + radius / 2, centerY - radius / 4,
                             centerX + radius, centerY - radius / 2,
                             foreground);
    Watchy::display.drawLine(centerX - radius / 2, centerY + radius / 4,
                             centerX - radius, centerY + radius / 2,
                             foreground);
    Watchy::display.drawLine(centerX + radius / 2, centerY + radius / 4,
                             centerX + radius, centerY + radius / 2,
                             foreground);
    break;
  case StatusIcon::SENSOR:
    Watchy::display.drawCircle(centerX, centerY, 3, foreground);
    Watchy::display.drawLine(centerX - radius, centerY, centerX + radius,
                             centerY, foreground);
    Watchy::display.drawLine(centerX, centerY - radius, centerX,
                             centerY + radius, foreground);
    Watchy::display.drawLine(centerX - radius + 2, centerY + radius - 2,
                             centerX + radius - 2, centerY - radius + 2,
                             foreground);
    break;
  case StatusIcon::TIME:
    Watchy::display.drawCircle(centerX, centerY, radius, foreground);
    Watchy::display.drawLine(centerX, centerY, centerX,
                             centerY - radius / 2, foreground);
    Watchy::display.drawLine(centerX, centerY, centerX + radius / 2,
                             centerY + 2, foreground);
    break;
  default: {
    Watchy::display.drawCircle(centerX, centerY, radius, foreground);
    const char *symbol = "i";
    if (icon == StatusIcon::SUCCESS) symbol = "+";
    if (icon == StatusIcon::ERROR) symbol = "x";
    if (icon == StatusIcon::LOADING) symbol = "...";
    if (icon == StatusIcon::EMPTY) symbol = "-";
    if (icon == StatusIcon::INFO && emphasized) {
      Watchy::display.fillCircle(centerX, centerY, radius - 2, foreground);
      Watchy::display.setTextColor(background);
      WatchyUi::Canvas::centeredText(bounds, symbol, 2, background);
      break;
    }
    centered(bounds, symbol, icon == StatusIcon::LOADING ? 1 : 2);
    break;
  }
  }
}

void drawProgressTrack(const WatchyUi::Bounds &bounds, float progress,
                       bool reverse, bool marker) {
  if (bounds.width < 8 || bounds.height < 5) return;
  progress = clamped(progress);
  const uint16_t foreground = WatchyUi::Theme::foreground();
  WatchyUi::GrayPaint::fillRoundRect(
      bounds, min<int16_t>(bounds.height / 2, 4),
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  Watchy::display.drawRoundRect(bounds.x, bounds.y, bounds.width,
                                bounds.height,
                                min<int16_t>(bounds.height / 2, 4), foreground);
  int16_t fillWidth = static_cast<int16_t>((bounds.width - 4) * progress);
  if (fillWidth > 0) {
    int16_t fillX = reverse ? bounds.x + bounds.width - 2 - fillWidth
                            : bounds.x + 2;
    Watchy::display.fillRect(fillX, bounds.y + 2, fillWidth,
                              max<int16_t>(1, bounds.height - 4), foreground);
  }
  if (marker) {
    int16_t markerX = bounds.x + 2 +
                      static_cast<int16_t>((bounds.width - 4) * progress);
    Watchy::display.drawLine(markerX, bounds.y - 2, markerX,
                             bounds.y + bounds.height + 1, foreground);
  }
}

void drawMetric(const WatchyUi::Bounds &bounds, const char *label,
                const char *value, float progress, const char *detail) {
  if (bounds.width <= 0 || bounds.height <= 0) return;
  WatchyUi::GrayPaint::fillRoundRect(
      bounds, 4, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  centered({bounds.x, bounds.y, bounds.width, 17}, label);

  int16_t valueTop = bounds.y + 17;
  int16_t valueHeight = detail == nullptr ? bounds.height - 25
                                           : bounds.height - 39;
  centered({static_cast<int16_t>(bounds.x + kInset), valueTop,
            static_cast<int16_t>(bounds.width - 2 * kInset), valueHeight},
           value, textSizeForValue(value));

  if (detail != nullptr && detail[0] != '\0') {
    centered({static_cast<int16_t>(bounds.x + 4),
              static_cast<int16_t>(bounds.y + bounds.height - 25),
              static_cast<int16_t>(bounds.width - 8), 12}, detail);
  }
  if (progress >= 0.0f) {
    drawProgressTrack({static_cast<int16_t>(bounds.x + 8),
                       static_cast<int16_t>(bounds.y + bounds.height - 10),
                       static_cast<int16_t>(bounds.width - 16), 6}, progress);
  }
}

void drawSignalBars(const WatchyUi::Bounds &bounds, uint8_t strength,
                    uint8_t maximum, bool framed) {
  if (maximum == 0 || bounds.width < maximum * 3) return;
  strength = min(strength, maximum);
  const uint16_t foreground = WatchyUi::Theme::foreground();
  if (framed) {
    WatchyUi::GrayPaint::fillRoundRect(
        bounds, 4, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  }
  constexpr int16_t gap = 3;
  int16_t availableWidth = bounds.width - gap * (maximum - 1) - 8;
  int16_t barWidth = max<int16_t>(2, availableWidth / maximum);
  int16_t usedWidth = barWidth * maximum + gap * (maximum - 1);
  int16_t x = bounds.x + (bounds.width - usedWidth) / 2;
  for (uint8_t index = 0; index < maximum; index++) {
    int16_t height = 5 + (bounds.height - 9) * (index + 1) / maximum;
    int16_t y = bounds.y + bounds.height - 4 - height;
    Watchy::display.drawRect(x, y, barWidth, height, foreground);
    if (index < strength) {
      Watchy::display.fillRect(x + 1, y + 1, max<int16_t>(1, barWidth - 2),
                                max<int16_t>(1, height - 2), foreground);
    }
    x += barWidth + gap;
  }
}

void drawMiniChart(const WatchyUi::Bounds &bounds, const int16_t *values,
                   uint8_t count, int16_t minimum, int16_t maximum,
                   bool fill) {
  if (values == nullptr || count < 2 || maximum <= minimum ||
      bounds.width < 8 || bounds.height < 8) {
    return;
  }
  WatchyUi::GrayPaint::fillRoundRect(
      bounds, 3, WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  const uint16_t foreground = WatchyUi::Theme::foreground();
  for (uint8_t grid = 1; grid < 4; grid++) {
    int16_t y = bounds.y + grid * (bounds.height - 2) / 4;
    WatchyUi::GrayPaint::line(bounds.x + 2, y, bounds.x + bounds.width - 3,
                               y, WatchyUi::Theme::tone(WatchyUi::ToneRole::Separator));
  }
  int16_t previousX = 0;
  int16_t previousY = 0;
  for (uint8_t index = 0; index < count; index++) {
    float normalized = clamped(static_cast<float>(values[index] - minimum) /
                               static_cast<float>(maximum - minimum));
    int16_t x = bounds.x + 2 + index * (bounds.width - 5) / (count - 1);
    int16_t y = bounds.y + bounds.height - 3 -
                static_cast<int16_t>(normalized * (bounds.height - 6));
    if (index > 0) {
      Watchy::display.drawLine(previousX, previousY, x, y, foreground);
    }
    if (fill) {
      WatchyUi::GrayPaint::line(x, y, x, bounds.y + bounds.height - 3,
                                 WatchyUi::Theme::tone(WatchyUi::ToneRole::Selection));
    }
    previousX = x;
    previousY = y;
  }
}

void drawTimeline(const WatchyUi::Bounds &bounds, float start, float end,
                  float marker) {
  start = clamped(start);
  end = clamped(end);
  if (end < start) {
    float temporary = start;
    start = end;
    end = temporary;
  }
  WatchyUi::GrayPaint::fillRoundRect(
      bounds, min<int16_t>(bounds.height / 2, 4),
      WatchyUi::Theme::tone(WatchyUi::ToneRole::SurfaceRaised));
  const uint16_t foreground = WatchyUi::Theme::foreground();
  int16_t startX = bounds.x + static_cast<int16_t>(start * (bounds.width - 1));
  int16_t endX = bounds.x + static_cast<int16_t>(end * (bounds.width - 1));
  if (endX > startX) {
    Watchy::display.fillRect(startX, bounds.y + 2, endX - startX,
                              max<int16_t>(1, bounds.height - 4), foreground);
  }
  Watchy::display.drawLine(bounds.x, bounds.y + bounds.height / 2,
                           bounds.x + bounds.width - 1,
                           bounds.y + bounds.height / 2, foreground);
  if (marker >= 0.0f) {
    int16_t markerX = bounds.x +
                      static_cast<int16_t>(clamped(marker) * (bounds.width - 1));
    Watchy::display.drawLine(markerX, bounds.y - 3, markerX,
                             bounds.y + bounds.height + 2, foreground);
  }
}

int16_t centeredCursorY(int16_t centerY, const char *text) {
  const char *safe = text == nullptr ? "" : text;
  if (safe[0] == '\0') return centerY;
  int16_t textX;
  int16_t textY;
  uint16_t textWidth;
  uint16_t textHeight;
  Watchy::display.getTextBounds(safe, 0, 0, &textX, &textY, &textWidth,
                                &textHeight);
  return static_cast<int16_t>(centerY - static_cast<int16_t>(textHeight) / 2 -
                              textY);
}

void drawDataRow(int16_t y, const char *label, const char *value,
                 bool emphasized) {
  constexpr int16_t rowX = 8;
  constexpr int16_t rowWidth = 184;
  constexpr int16_t rowHeight = 17;
  const int16_t rowCenterY = static_cast<int16_t>(y - 12 + rowHeight / 2);
  if (emphasized) {
    WatchyUi::GrayPaint::fillRoundRect(
        {rowX, static_cast<int16_t>(y - 12), rowWidth, rowHeight}, 3,
        WatchyUi::Theme::tone(WatchyUi::ToneRole::Surface));
  }
  Watchy::display.setFont();
  Watchy::display.setTextSize(1);
  Watchy::display.setTextColor(WatchyUi::Theme::foreground());
  const char *safeLabel = label == nullptr ? "" : label;
  int16_t textX;
  int16_t textY;
  uint16_t labelWidth;
  uint16_t labelHeight;
  uint16_t valueWidth;
  uint16_t valueHeight;
  Watchy::display.getTextBounds(safeLabel, 0, 0, &textX, &textY, &labelWidth,
                                &labelHeight);
  Watchy::display.setCursor(rowX + 5, centeredCursorY(rowCenterY, safeLabel));
  Watchy::display.print(safeLabel);
  const char *safeValue = value == nullptr ? "" : value;
  char clippedValue[25];
  size_t length = min<size_t>(strlen(safeValue), sizeof(clippedValue) - 1);
  memcpy(clippedValue, safeValue, length);
  clippedValue[length] = '\0';
  int16_t maximumValueWidth = max<int16_t>(
      12, rowWidth - 15 - static_cast<int16_t>(labelWidth));
  while (length > 0) {
    Watchy::display.getTextBounds(clippedValue, 0, 0, &textX, &textY,
                                  &valueWidth, &valueHeight);
    if (valueWidth <= maximumValueWidth) break;
    if (length > 3) {
      clippedValue[length - 1] = '.';
      clippedValue[length - 2] = '.';
      clippedValue[length - 3] = '.';
    }
    clippedValue[--length] = '\0';
  }
  Watchy::display.getTextBounds(clippedValue, 0, 0, &textX, &textY,
                                &valueWidth, &valueHeight);
  int16_t valueX = max<int16_t>(rowX + 10 + labelWidth,
                                rowX + rowWidth - 5 - valueWidth);
  Watchy::display.setCursor(valueX, centeredCursorY(rowCenterY, clippedValue));
  Watchy::display.print(clippedValue);
}

void drawEmptyState(const WatchyUi::Bounds &bounds, const char *label,
                    const char *detail) {
  int16_t iconSize = min<int16_t>(42, bounds.width / 3);
  int16_t iconX = bounds.x + (bounds.width - iconSize) / 2;
  drawStatusIcon({iconX, static_cast<int16_t>(bounds.y + 4), iconSize,
                  iconSize}, StatusIcon::EMPTY, true);
  centered({bounds.x, static_cast<int16_t>(bounds.y + iconSize + 12),
            bounds.width, 18}, label, strlen(label) > 15 ? 1 : 2);
  if (detail != nullptr) {
    centered({static_cast<int16_t>(bounds.x + 8),
          static_cast<int16_t>(bounds.y + iconSize + 35),
          static_cast<int16_t>(bounds.width - 16), 16}, detail);
  }
}

void drawWarningState(const WatchyUi::Bounds &bounds, const char *label,
                      const char *detail) {
  int16_t iconSize = min<int16_t>(42, bounds.width / 3);
  int16_t iconX = bounds.x + (bounds.width - iconSize) / 2;
  drawStatusIcon({iconX, static_cast<int16_t>(bounds.y + 4), iconSize,
                  iconSize},
                 StatusIcon::WARNING, true);
  centered({bounds.x, static_cast<int16_t>(bounds.y + iconSize + 12),
            bounds.width, 18}, label, strlen(label) > 15 ? 1 : 2);
  if (detail != nullptr) {
    centered({static_cast<int16_t>(bounds.x + 8),
          static_cast<int16_t>(bounds.y + iconSize + 35),
          static_cast<int16_t>(bounds.width - 16), 16}, detail);
  }
}

} // namespace AppVisual
