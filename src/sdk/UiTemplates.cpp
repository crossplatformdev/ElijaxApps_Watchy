#include "UiTemplates.h"

#include <Preferences.h>

#include "NetUtils.h"

#include <cstring>
#include <utility>

namespace UiTemplates {

static volatile ControlEvent gControlEvent = ControlEvent::None;
static volatile uint8_t gCaptureControlsDepth = 0;

void postControlEvent(ControlEvent ev) {
  gControlEvent = ev;
}

ControlEvent takeControlEvent() {
  ControlEvent ev = gControlEvent;
  gControlEvent = ControlEvent::None;
  return ev;
}

void clearControlEvent() {
  gControlEvent = ControlEvent::None;
}

void beginControlCapture() {
  if (gCaptureControlsDepth < 255) {
    ++gCaptureControlsDepth;
  }
}

void endControlCapture() {
  if (gCaptureControlsDepth > 0) {
    --gCaptureControlsDepth;
  }
}

bool isCapturingControls() {
  return gCaptureControlsDepth > 0;
}

namespace {
void waitForAllButtonsReleasedInternal(uint32_t stableMs, uint32_t timeoutMs);
} // namespace

void waitForAllButtonsReleased(uint32_t stableMs, uint32_t timeoutMs) {
  waitForAllButtonsReleasedInternal(stableMs, timeoutMs);
}

namespace {

void waitForAllButtonsReleasedInternal(uint32_t stableMs = 40, uint32_t timeoutMs = 600) {
  const uint32_t startMs = millis();
  uint32_t stableStartMs = 0;
  while (millis() - startMs < timeoutMs) {
    const bool anyPressed = (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) ||
                            (digitalRead(UP_BTN_PIN) == ACTIVE_LOW) ||
                            (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW) ||
                            (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
    if (anyPressed) {
      stableStartMs = 0;
    } else {
      if (stableStartMs == 0) {
        stableStartMs = millis();
      }
      if (millis() - stableStartMs >= stableMs) {
        break;
      }
    }
    delay(5);
  }
}

static bool isDisabledLabel(const char *label) {
  return label == nullptr || label[0] == '\0' || (label[0] == '-' && label[1] == '\0');
}

static void setFixedRowControls(UIAppSpec &app,
                               const char *backLabel,
                               const char *upLabel,
                               const char *acceptLabel,
                               const char *downLabel) {
  app.controls[0].label = backLabel;
  app.controls[0].callback = isDisabledLabel(backLabel) ? nullptr : &Watchy::backPressed;
  app.controls[1].label = upLabel;
  app.controls[1].callback = isDisabledLabel(upLabel) ? nullptr : &Watchy::upPressed;
  app.controls[2].label = acceptLabel;
  app.controls[2].callback = isDisabledLabel(acceptLabel) ? nullptr : &Watchy::menuPressed;
  app.controls[3].label = downLabel;
  app.controls[3].callback = isDisabledLabel(downLabel) ? nullptr : &Watchy::downPressed;
}

static constexpr const char *kCountKey = "n";

static void makeItemKey(char out[4], uint8_t idx) {
  // keys must be <= 15 chars for Preferences/NVS.
  // h00 .. h41
  out[0] = 'h';
  out[1] = '0' + (idx / 10);
  out[2] = '0' + (idx % 10);
  out[3] = '\0';
}

static uint8_t clampMaxEntries(uint8_t maxEntries) {
  if (maxEntries == 0) return 1;
  if (maxEntries > HISTORY_MAX_ENTRIES) return HISTORY_MAX_ENTRIES;
  return maxEntries;
}

static uint16_t clampMaxEntryLen(uint16_t maxLen) {
  if (maxLen == 0) return 1;
  if (maxLen > HISTORY_MAX_ENTRY_LEN) return HISTORY_MAX_ENTRY_LEN;
  return maxLen;
}

static uint8_t loadHistoryFromNvs(const char *ns,
                                  String *outItems,
                                  uint8_t maxItems,
                                  uint16_t maxLen) {
  if (ns == nullptr || ns[0] == '\0' || outItems == nullptr) {
    return 0;
  }

  Preferences prefs;
  if (!prefs.begin(ns, false)) {
    return 0;
  }

  const uint8_t limit = clampMaxEntries(maxItems);
  const uint16_t lenLimit = clampMaxEntryLen(maxLen);
  uint8_t count = prefs.getUChar(kCountKey, 0);
  if (count > limit) {
    count = limit;
  }

  char key[4];
  for (uint8_t i = 0; i < count; ++i) {
    makeItemKey(key, i);
    outItems[i] = prefs.getString(key, "");
    if (outItems[i].length() > lenLimit) {
      outItems[i].remove(lenLimit);
    }
  }

  // Ensure any extra slots are empty.
  for (uint8_t i = count; i < limit; ++i) {
    outItems[i] = "";
  }

  prefs.end();
  return count;
}

static void saveHistoryToNvs(const char *ns,
                             const String *items,
                             uint8_t count,
                             uint8_t maxItems) {
  if (ns == nullptr || ns[0] == '\0' || items == nullptr) {
    return;
  }

  Preferences prefs;
  if (!prefs.begin(ns, false)) {
    return;
  }

  const uint8_t limit = clampMaxEntries(maxItems);
  if (count > limit) {
    count = limit;
  }

  prefs.putUChar(kCountKey, count);
  char key[4];
  for (uint8_t i = 0; i < count; ++i) {
    makeItemKey(key, i);
    prefs.putString(key, items[i]);
  }

  // Clear remaining keys from old histories.
  for (uint8_t i = count; i < limit; ++i) {
    makeItemKey(key, i);
    prefs.remove(key);
  }

  prefs.end();
}

static bool historyContains(const String *items, uint8_t count, const char *value) {
  if (items == nullptr || value == nullptr) {
    return false;
  }
  for (uint8_t i = 0; i < count; ++i) {
    if (items[i].equals(value)) {
      return true;
    }
  }
  return false;
}

static void deleteHistoryAt(String *items, uint8_t &count, uint8_t idx, uint8_t maxItems) {
  const uint8_t limit = clampMaxEntries(maxItems);
  if (idx >= count || items == nullptr) {
    return;
  }
  for (uint8_t i = idx; i + 1 < count; ++i) {
    items[i] = items[i + 1];
  }
  if (count > 0) {
    count--;
  }
  for (uint8_t i = count; i < limit; ++i) {
    items[i] = "";
  }
}

static void addUnique(String *items,
                      uint8_t &count,
                      uint8_t maxItems,
                      const char *value,
                      uint16_t maxLen) {
  if (items == nullptr || value == nullptr || value[0] == '\0') {
    return;
  }

  const uint8_t limit = clampMaxEntries(maxItems);
  const uint16_t lenLimit = clampMaxEntryLen(maxLen);

  // Unique check.
  for (uint8_t i = 0; i < count; ++i) {
    if (items[i].equals(value)) {
      return;
    }
  }

  String v(value);
  if (v.length() > lenLimit) {
    v.remove(lenLimit);
  }

  if (count < limit) {
    items[count++] = v;
    return;
  }

  // Drop oldest.
  for (uint8_t i = 0; i + 1 < limit; ++i) {
    items[i] = items[i + 1];
  }
  items[limit - 1] = v;
  count = limit;
}

} // namespace

int8_t runToast2Option(Watchy &watchy,
                       const char *title,
                       const char *option0,
                       const char *option1,
                       int8_t defaultSelection,
                       const ToastLayout &layout,
                       const char *backLabel,
                       const char *upLabel,
                       const char *acceptLabel,
                       const char *downLabel) {
  int8_t selected = (defaultSelection == 0) ? 0 : 1;
  // Draw overlay rectangle.
  const uint16_t bg = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t fg = UiSDK::getWatchfaceFg(BASE_POLARITY);

  watchy.display.setFont(layout.font);
  watchy.display.setTextColor(fg);

  constexpr int16_t kToastCornerRadius = 8;
  watchy.display.fillRoundRect(layout.x, layout.y, layout.w, layout.h, kToastCornerRadius, bg);
  watchy.display.drawRoundRect(layout.x, layout.y, layout.w, layout.h, kToastCornerRadius, fg);
  
  if (title) {
    watchy.display.setCursor(layout.titleX, layout.titleY);
    watchy.display.print(title);
  }

  auto drawOption = [&](int8_t idx, const char *text, int16_t x, int16_t y) {
  if (text == nullptr) return;
  if (selected == idx) {
    // Simple highlight bar.
    watchy.display.fillRect(layout.x + 4, y - 12, layout.w - 8, 16, fg);
    watchy.display.setTextColor(bg);
  } else {
    // Clear any previous highlight behind this option.
    watchy.display.fillRect(layout.x + 4, y - 12, layout.w - 8, 16, bg);
    watchy.display.setTextColor(fg);
  }
    watchy.display.setCursor(x, y);
    watchy.display.print(text);
    watchy.display.setTextColor(fg);
  };

  drawOption(0, option0, layout.option0X, layout.option0Y);
  drawOption(1, option1, layout.option1X, layout.option1Y);

  beginControlCapture();
  clearControlEvent();
  // Avoid immediately accepting/toggling due to a carried-over button press.
  waitForAllButtonsReleased();

  UIControlsRowLayout controls[4] = {
      {backLabel, &Watchy::backPressed},
      {upLabel, &Watchy::upPressed},
      {acceptLabel, &Watchy::menuPressed},
      {downLabel, &Watchy::downPressed},
  };
  
  while (true) {
    watchy.display.display(true);

    UiSDK::renderControlsRow(watchy, controls);
    const ControlEvent ev = takeControlEvent();
    if (ev == ControlEvent::None) {
      delay(10);
      continue;
    }

    if (ev == ControlEvent::Back) {
      endControlCapture();
      return -1;
    }

    if (ev == ControlEvent::Accept) {
      endControlCapture();
      return selected;
    }

    if (ev == ControlEvent::Up || ev == ControlEvent::Down) {
      selected = (selected == 0) ? 1 : 0;
      // redraw highlight
      drawOption(0, option0, layout.option0X, layout.option0Y);
      drawOption(1, option1, layout.option1X, layout.option1Y);
      continue;
    }
  }
}

void renderControlsRowAt(WatchyGxDisplay &display,
                         const char *backLabel,
                         const char *upLabel,
                         const char *acceptLabel,
                         const char *downLabel,
                         const ControlsRowLayout &layout) {
  if (backLabel) {
    display.setCursor(layout.backX, layout.backY);
    display.print(backLabel);
  }
  if (upLabel) {
    display.setCursor(layout.upX, layout.upY);
    display.print(upLabel);
  }
  if (acceptLabel) {
    display.setCursor(layout.acceptX, layout.acceptY);
    display.print(acceptLabel);
  }
  if (downLabel) {
    display.setCursor(layout.downX, layout.downY);
    display.print(downLabel);
  }
}

void renderMenuPickerPage(Watchy &watchy,
                          const char *title,
                          const UIMenuItemSpec *items,
                          uint8_t itemCount,
                          int8_t selectedIndex,
                          const MenuPickerLayout &layout,
                          const char *backLabel,
                          const char *upLabel,
                          const char *acceptLabel,
                          const char *downLabel) {
  if (itemCount == 0 || items == nullptr) {
    return;
  }

  int8_t selected = selectedIndex;
  if (selected < 0) selected = 0;
  if (selected >= static_cast<int8_t>(itemCount)) selected = static_cast<int8_t>(itemCount - 1);

  UITextSpec header{};
  header.x = layout.headerX;
  header.y = layout.headerY;
  header.font = layout.font;
  header.text = title;

  UIMenuSpec menu{};
  menu.x = layout.menuX;
  menu.y = layout.menuY;
  menu.w = WatchyDisplay::WIDTH;
  menu.itemHeight = MENU_HEIGHT;
  menu.font = layout.font;
  menu.items = items;
  menu.itemCount = itemCount;
  menu.selectedIndex = selected;

  const uint8_t visible = (itemCount < layout.visibleRowsMax) ? itemCount : layout.visibleRowsMax;
  menu.visibleCount = visible;
  menu.startIndex = layout.autoScroll ? calcMenuStartIndex(static_cast<uint8_t>(selected), visible, itemCount)
                    : layout.startIndex;

  UIAppSpec app{};
  app.texts = &header;
  app.textCount = 1;
  app.menus = &menu;
  app.menuCount = 1;

  setFixedRowControls(app, backLabel, upLabel, acceptLabel, downLabel);

  UiSDK::renderApp(watchy, app);
}

void renderCenteredMonospaceHighlight(WatchyGxDisplay &display,
                                     const GFXfont *font,
                                     const char *text,
                                     uint8_t textLen,
                                     int16_t y,
                                     int8_t highlightIndex,
                                     uint16_t fg,
                                     uint16_t bg,
                                     int16_t highlightExtraHeight) {
  if (font == nullptr || text == nullptr || textLen == 0) {
    return;
  }

  display.setFont(font);

  int16_t x1, y1;
  uint16_t cw, ch;
  display.getTextBounds("0", 0, 0, &x1, &y1, &cw, &ch);
  const int16_t width = static_cast<int16_t>(cw) * static_cast<int16_t>(textLen);
  const int16_t baseX = (WatchyDisplay::WIDTH - width) / 2;

  for (uint8_t i = 0; i < textLen; ++i) {
    const char c = text[i];
    const int16_t x = baseX + static_cast<int16_t>(i) * static_cast<int16_t>(cw);

    if (static_cast<int8_t>(i) == highlightIndex) {
      display.fillRect(x, y - static_cast<int16_t>(ch), cw, static_cast<int16_t>(ch) + highlightExtraHeight, fg);
      display.setTextColor(bg);
    } else {
      display.setTextColor(fg);
    }

    display.setCursor(x, y);
    display.print(c);
  }
  display.setTextColor(fg);
}

void renderBarePage(Watchy &watchy, const UIAppSpec &app) {
  UIAppSpec withChrome = app;

  withChrome.controls[0].label = nullptr;
  withChrome.controls[0].callback = nullptr;
  withChrome.controls[1].label = nullptr;
  withChrome.controls[1].callback = nullptr;
  withChrome.controls[2].label = nullptr;
  withChrome.controls[2].callback = nullptr;
  withChrome.controls[3].label = nullptr;
  withChrome.controls[3].callback = nullptr;
  UiSDK::renderApp(watchy, withChrome);
}

void renderPageWithControlsAt(Watchy &watchy,
                              const UIAppSpec &app,
                              const char *backLabel,
                              const char *upLabel,
                              const char *acceptLabel,
                              const char *downLabel,
                              const ControlsRowLayout &controlsLayout) {
  UIAppSpec withChrome = app;
  (void)controlsLayout;
  // This helper is render-only; the actual coordinate-based row should be
  // drawn by callers using renderControlsRowAt(). We still populate the fixed
  // row hints so screens remain consistent when used with UiSDK::renderApp.
  setFixedRowControls(withChrome, backLabel, upLabel, acceptLabel, downLabel);
  UiSDK::renderApp(watchy, withChrome);
}

uint8_t calcMenuStartIndex(uint8_t selectedIndex, uint8_t visibleRows, uint8_t itemCount) {
  if (visibleRows == 0 || itemCount == 0) {
    return 0;
  }
  if (itemCount <= visibleRows) {
    return 0;
  }
  if (selectedIndex >= visibleRows) {
    uint8_t start = selectedIndex - visibleRows + 1;
    if (start + visibleRows > itemCount) {
      start = itemCount - visibleRows;
    }
    return start;
  }
  return 0;
}

void keepMenuSelectionVisible(uint8_t selectedIndex,
                              uint8_t visibleRows,
                              uint8_t itemCount,
                              uint8_t &inOutStartIndex) {
  if (visibleRows == 0 || itemCount == 0 || itemCount <= visibleRows) {
    inOutStartIndex = 0;
    return;
  }

  if (selectedIndex < inOutStartIndex) {
    inOutStartIndex = selectedIndex;
  } else if (selectedIndex >= static_cast<uint8_t>(inOutStartIndex + visibleRows)) {
    inOutStartIndex = selectedIndex - visibleRows + 1;
  }

  if (inOutStartIndex + visibleRows > itemCount) {
    inOutStartIndex = itemCount - visibleRows;
  }
}

int8_t runMenuPicker(Watchy &watchy,
                     const char *title,
                     const UIMenuItemSpec *items,
                     uint8_t itemCount,
                     int8_t &inOutSelectedIndex,
                     const MenuPickerLayout &layout,
                     const char *backLabel,
                     const char *upLabel,
                     const char *acceptLabel,
                     const char *downLabel) {
  if (itemCount == 0 || items == nullptr) {
    return -1;
  }

  int8_t selected = inOutSelectedIndex;
  if (selected < 0) selected = 0;
  if (selected >= static_cast<int8_t>(itemCount)) selected = static_cast<int8_t>(itemCount - 1);

  beginControlCapture();
  clearControlEvent();
  // Prevent a carried-over press (e.g., from the app launcher menu) from
  // immediately triggering ACCEPT on entry.
  waitForAllButtonsReleased();

  UITextSpec header{};
  header.x = layout.headerX;
  header.y = layout.headerY;
  header.font = layout.font;
  header.text = title;

  UIMenuSpec menu{};
  menu.x = layout.menuX;
  menu.y = layout.menuY;
  menu.w = WatchyDisplay::WIDTH;
  menu.itemHeight = MENU_HEIGHT;
  menu.font = layout.font;
  menu.items = items;
  menu.itemCount = itemCount;
  menu.selectedIndex = selected;

  const uint8_t visible = (itemCount < layout.visibleRowsMax) ? itemCount : layout.visibleRowsMax;
  menu.visibleCount = visible;

  menu.startIndex = layout.autoScroll ? calcMenuStartIndex(static_cast<uint8_t>(selected), visible, itemCount)
                    : layout.startIndex;

  UIAppSpec app{};
  app.texts = &header;
  app.textCount = 1;
  app.menus = &menu;
  app.menuCount = 1;

  setFixedRowControls(app, backLabel, upLabel, acceptLabel, downLabel);

  while (true) {
    menu.selectedIndex = selected;
    if (layout.autoScroll) {
      menu.startIndex = calcMenuStartIndex(static_cast<uint8_t>(selected), visible, itemCount);
    }

    clearControlEvent();
    UiSDK::renderApp(watchy, app);

    while (true) {
      UiSDK::renderControlsRow(watchy, app.controls);
      const ControlEvent ev = takeControlEvent();
      if (ev == ControlEvent::None) {
        delay(10);
        continue;
      }

      if (ev == ControlEvent::Back) {
        endControlCapture();
        return -1;
      }
      if (ev == ControlEvent::Up) {
        selected = (selected > 0) ? (selected - 1) : (itemCount - 1);
        break;
      }
      if (ev == ControlEvent::Down) {
        selected = (selected + 1 >= itemCount) ? 0 : (selected + 1);
        break;
      }
      if (ev == ControlEvent::Accept) {
        inOutSelectedIndex = selected;
        endControlCapture();
        return selected;
      }
    }
  }
}

bool runHistoryPickerNvs(Watchy &watchy,
                         const HistoryPickerSpec &spec,
                         char *outValue,
                         size_t outValueSize,
                         HistoryPickKind &outKind,
                         int8_t &inOutSelectedIndex) {
  if (outValue == nullptr || outValueSize == 0) {
    return false;
  }
  outValue[0] = '\0';

  const uint8_t limit = clampMaxEntries(spec.maxEntries);
  const uint16_t lenLimit = clampMaxEntryLen(spec.maxEntryLen);

  // Keep these alive for menu label pointers.
  String history[HISTORY_MAX_ENTRIES];
  uint8_t count = loadHistoryFromNvs(spec.nvsNamespace, history, limit, lenLimit);

  int8_t selected = inOutSelectedIndex;
  // If caller passes -1, we default to the first entry (never to "New target").

  while (true) {
    const bool hasExample = (spec.exampleValue && spec.exampleValue[0])
                                ? historyContains(history, count, spec.exampleValue)
                                : true;

    static UIMenuItemSpec items[HISTORY_MAX_ENTRIES + 2];
    uint8_t total = 0;
    for (uint8_t i = 0; i < count && total < (HISTORY_MAX_ENTRIES + 2); ++i) {
      items[total++].label = history[i].c_str();
    }

    const int8_t exampleIndex = (!hasExample && spec.exampleValue && spec.exampleValue[0])
                                    ? static_cast<int8_t>(total)
                                    : -1;
    if (!hasExample && spec.exampleValue && spec.exampleValue[0] && total < (HISTORY_MAX_ENTRIES + 2)) {
      const char *exampleLabel = (spec.exampleLabel && spec.exampleLabel[0]) ? spec.exampleLabel : spec.exampleValue;
      items[total++].label = exampleLabel;
    }

    const int8_t newIndex = static_cast<int8_t>(total);
    if (total < (HISTORY_MAX_ENTRIES + 2)) {
      items[total++].label = (spec.newLabel ? spec.newLabel : "New target");
    }

    if (selected < 0) {
      selected = 0;
    }

    if (total == 0) {
      return false;
    }
    if (selected >= static_cast<int8_t>(total)) {
      selected = static_cast<int8_t>(total) - 1;
    }

    const int8_t chosen = runMenuPicker(watchy,
                                        (spec.title ? spec.title : "History"),
                                        items,
                                        total,
                                        selected,
                                        spec.menuLayout,
                                        "BACK",
                                        "UP",
                                        "ACCEPT",
                                        "DOWN");
    if (chosen < 0) {
      const int8_t action = runToast2Option(watchy,
                                            (spec.backPromptTitle ? spec.backPromptTitle : "History"),
                                            (spec.deleteRecordLabel ? spec.deleteRecordLabel : "Delete record"),
                                            (spec.exitAppLabel ? spec.exitAppLabel : "Exit App"));
      if (action < 0) {
        continue;
      }
      if (action == 0) {
        // Delete highlighted item only if it's an actual history item.
        if (selected >= 0 && selected < static_cast<int8_t>(count)) {
          deleteHistoryAt(history, count, static_cast<uint8_t>(selected), limit);
          saveHistoryToNvs(spec.nvsNamespace, history, count, limit);
          if (selected >= static_cast<int8_t>(count)) {
            selected = static_cast<int8_t>(count) - 1;
          }
          if (selected < 0) selected = 0;
        }
        continue;
      }
      return false;
    }

    inOutSelectedIndex = selected;

    if (chosen < static_cast<int8_t>(count)) {
      strncpy(outValue, history[chosen].c_str(), outValueSize - 1);
      outValue[outValueSize - 1] = '\0';
      outKind = HistoryPickKind::HistoryItem;
      return true;
    }

    if (exampleIndex >= 0 && chosen == exampleIndex) {
      strncpy(outValue, spec.exampleValue, outValueSize - 1);
      outValue[outValueSize - 1] = '\0';
      outKind = HistoryPickKind::ExampleQuickPick;
      return true;
    }

    if (chosen == newIndex) {
      outValue[0] = '\0';
      outKind = HistoryPickKind::NewTarget;
      return true;
    }

    outValue[0] = '\0';
    outKind = HistoryPickKind::NewTarget;
    return true;
  }
}

bool runHistoryPickerEditAndPersistNvs(Watchy &watchy,
                                      const HistoryPickerSpec &spec,
                                      char *inOutValue,
                                      size_t inOutValueSize,
                                      const char *editorTitle,
                                      int8_t &inOutSelectedIndex) {
  HistoryPickKind pickKind = HistoryPickKind::NewTarget;
  if (!runHistoryPickerNvs(watchy, spec, inOutValue, inOutValueSize, pickKind, inOutSelectedIndex)) {
    return false; // Exit App from history list
  }

  const String originalTarget = String(inOutValue);
  const char *title = (editorTitle && editorTitle[0]) ? editorTitle
                    : (spec.title ? spec.title : "Target");

  if (!NetUtils::editTarget(watchy, inOutValue, inOutValueSize, title)) {
    return false;
  }

  if (inOutValue[0] != '\0' && String(inOutValue) != originalTarget) {
    historyAddUniqueNvs(spec.nvsNamespace, inOutValue, spec.maxEntries, spec.maxEntryLen);
  }
  return true;
}

void historyAddUniqueNvs(const char *nvsNamespace, const char *value, uint8_t maxEntries, uint16_t maxEntryLen) {
  if (nvsNamespace == nullptr || nvsNamespace[0] == '\0') {
    return;
  }
  if (value == nullptr || value[0] == '\0') {
    return;
  }

  const uint8_t limit = clampMaxEntries(maxEntries);
  const uint16_t lenLimit = clampMaxEntryLen(maxEntryLen);

  String history[HISTORY_MAX_ENTRIES];
  uint8_t count = loadHistoryFromNvs(nvsNamespace, history, limit, lenLimit);
  addUnique(history, count, limit, value, lenLimit);
  saveHistoryToNvs(nvsNamespace, history, count, limit);
}

ViewerAction runRefreshableScrollableViewer(Watchy &watchy,
                                           UIAppSpec &app,
                                           UIScrollableTextSpec &scroll,
                                           uint16_t &inOutFirstLine,
                                           const char *backLabel,
                                           const char *upLabel,
                                           const char *refreshLabel,
                                           const char *downLabel,
                                           BuildScrollableTextFn buildFn,
                                           void *userData,
                                           String &scratchText) {
  beginControlCapture();
  // Viewer owns scroll.firstLine and uses scratchText via textRef.
  scroll.textRef = &scratchText;

  // Ensure the controls row has callbacks wired (callers often pass an empty app
  // spec and rely on the viewer to define the row behavior).
  setFixedRowControls(app, backLabel, upLabel, refreshLabel, downLabel);

  const int16_t padding = 6;
  const int16_t contentH = scroll.h - padding * 2;
  const uint16_t lineH = (scroll.lineHeight > 0) ? static_cast<uint16_t>(scroll.lineHeight) : 10;

  uint8_t visibleLines = scroll.maxLines;
  if (visibleLines == 0) {
    visibleLines = (contentH > 0) ? static_cast<uint8_t>(contentH / lineH) : 1;
    if (visibleLines < 1) visibleLines = 1;
    scroll.maxLines = visibleLines;
  }

  // UIScrollableTextSpec: UP/DOWN hold-repeat should NOT be debounced.
  uint32_t upLastRepeatMs = 0;
  uint32_t downLastRepeatMs = 0;
  const uint32_t kScrollRepeatMs = 10;

  auto readStableUpDown = [&]() {
    bool up1 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
    bool down1 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
    if (up1 != down1) {
      delay(2);
      bool up2 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
      bool down2 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
      if (up2 != down2) {
        up1 = up2;
        down1 = down2;
      }
    }
    return std::pair<bool, bool>(up1, down1);
  };

  bool needsRebuild = true;
  uint16_t totalLines = 0;

  while (true) {
    if (needsRebuild) {
      if (buildFn) {
        scratchText = "";
        buildFn(watchy, userData, scratchText);
      }

      totalLines = UiSDK::countScrollableTextLines(scroll);
      if (totalLines <= visibleLines) {
        inOutFirstLine = 0;
      } else if (inOutFirstLine + visibleLines > totalLines) {
        inOutFirstLine = totalLines - visibleLines;
      }
      scroll.firstLine = inOutFirstLine;

      renderScrollablePage(watchy, app, backLabel, upLabel, refreshLabel, downLabel);
      clearControlEvent();
      needsRebuild = false;
    }

    UiSDK::renderControlsRow(watchy, app.controls);
    const ControlEvent ev = takeControlEvent();

    const auto upDownPair = readStableUpDown();
    const bool upHeld = upDownPair.first;
    const bool downHeld = upDownPair.second;
    const uint32_t nowMs = millis();

    if (ev == ControlEvent::Back) {
      endControlCapture();
      return ViewerAction::Back;
    }

    if (ev == ControlEvent::Accept) {
      needsRebuild = true;
      continue;
    }

    bool didScroll = false;

    if (ev == ControlEvent::Up) {
      upLastRepeatMs = nowMs;
      if (inOutFirstLine > 0) {
        --inOutFirstLine;
        didScroll = true;
      }
    }

    if (ev == ControlEvent::Down) {
      downLastRepeatMs = nowMs;
      if (inOutFirstLine + visibleLines < totalLines) {
        ++inOutFirstLine;
        didScroll = true;
      }
    }

    if (!upHeld) {
      upLastRepeatMs = 0;
    }
    if (!downHeld) {
      downLastRepeatMs = 0;
    }

    if (upHeld && inOutFirstLine > 0) {
      if (upLastRepeatMs == 0 || (nowMs - upLastRepeatMs >= kScrollRepeatMs)) {
        upLastRepeatMs = nowMs;
        --inOutFirstLine;
        didScroll = true;
      }
    }

    if (downHeld && (inOutFirstLine + visibleLines < totalLines)) {
      if (downLastRepeatMs == 0 || (nowMs - downLastRepeatMs >= kScrollRepeatMs)) {
        downLastRepeatMs = nowMs;
        ++inOutFirstLine;
        didScroll = true;
      }
    }

    if (didScroll) {
      scroll.firstLine = inOutFirstLine;
      renderScrollablePage(watchy, app, backLabel, upLabel, refreshLabel, downLabel);
      continue;
    }

    if (ev == ControlEvent::None) {
      delay(10);
    }
  }
}

void renderScrollablePage(Watchy &watchy,
                          const UIAppSpec &app,
                          const char *backLabel,
                          const char *upLabel,
                          const char *acceptLabel,
                          const char *downLabel) {
  UIAppSpec withChrome = app;

  setFixedRowControls(withChrome, backLabel, upLabel, acceptLabel, downLabel);

  UiSDK::renderApp(watchy, withChrome);
}

ViewerAction runScrollableViewer(Watchy &watchy,
                                 UIAppSpec &app,
                                 UIScrollableTextSpec &scroll,
                                 uint16_t &inOutFirstLine,
                                 const char *backLabel,
                                 const char *upLabel,
                                 const char *acceptLabel,
                                 const char *downLabel,
                                 bool acceptEnabled) {
  beginControlCapture();
  // Viewer owns scroll.firstLine.
  scroll.firstLine = inOutFirstLine;

  // Ensure the controls row has callbacks wired (callers often pass an empty app
  // spec and rely on the viewer to define the row behavior).
  setFixedRowControls(app, backLabel, upLabel, acceptLabel, downLabel);

  const int16_t padding = 6;
  const int16_t contentH = scroll.h - padding * 2;
  const uint16_t lineH = (scroll.lineHeight > 0) ? static_cast<uint16_t>(scroll.lineHeight) : 10;

  uint8_t visibleLines = scroll.maxLines;
  if (visibleLines == 0) {
    visibleLines = (contentH > 0) ? static_cast<uint8_t>(contentH / lineH) : 1;
    if (visibleLines < 1) visibleLines = 1;
    scroll.maxLines = visibleLines;
  }

  uint16_t totalLines = UiSDK::countScrollableTextLines(scroll);
  if (totalLines <= visibleLines) {
    inOutFirstLine = 0;
    scroll.firstLine = 0;
  } else if (inOutFirstLine + visibleLines > totalLines) {
    inOutFirstLine = totalLines - visibleLines;
    scroll.firstLine = inOutFirstLine;
  }

  renderScrollablePage(watchy, app, backLabel, upLabel, acceptLabel, downLabel);
  clearControlEvent();

  // UIScrollableTextSpec: UP/DOWN hold-repeat should NOT be debounced.
  uint32_t upLastRepeatMs = 0;
  uint32_t downLastRepeatMs = 0;
  const uint32_t kScrollRepeatMs = 10;

  auto readStableUpDown = [&]() {
    bool up1 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
    bool down1 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
    if (up1 != down1) {
      delay(2);
      bool up2 = (digitalRead(UP_BTN_PIN) == ACTIVE_LOW);
      bool down2 = (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW);
      if (up2 != down2) {
        up1 = up2;
        down1 = down2;
      }
    }
    return std::pair<bool, bool>(up1, down1);
  };

  while (true) {
    UiSDK::renderControlsRow(watchy, app.controls);
    const ControlEvent ev = takeControlEvent();

    const auto upDownPair = readStableUpDown();
    const bool upHeld = upDownPair.first;
    const bool downHeld = upDownPair.second;
    const uint32_t nowMs = millis();

    if (ev == ControlEvent::Back) {
      endControlCapture();
      return ViewerAction::Back;
    }

    if (ev == ControlEvent::Accept) {
      if (acceptEnabled) {
        endControlCapture();
        return ViewerAction::Accept;
      }
    }

    bool didScroll = false;

    if (ev == ControlEvent::Up) {
      upLastRepeatMs = nowMs;
      if (inOutFirstLine > 0) {
        --inOutFirstLine;
        didScroll = true;
      }
    }

    if (ev == ControlEvent::Down) {
      downLastRepeatMs = nowMs;
      if (inOutFirstLine + visibleLines < totalLines) {
        ++inOutFirstLine;
        didScroll = true;
      }
    }

    if (!upHeld) {
      upLastRepeatMs = 0;
    }
    if (!downHeld) {
      downLastRepeatMs = 0;
    }

    if (upHeld && inOutFirstLine > 0) {
      if (upLastRepeatMs == 0 || (nowMs - upLastRepeatMs >= kScrollRepeatMs)) {
        upLastRepeatMs = nowMs;
        --inOutFirstLine;
        didScroll = true;
      }
    }

    if (downHeld && (inOutFirstLine + visibleLines < totalLines)) {
      if (downLastRepeatMs == 0 || (nowMs - downLastRepeatMs >= kScrollRepeatMs)) {
        downLastRepeatMs = nowMs;
        ++inOutFirstLine;
        didScroll = true;
      }
    }

    if (didScroll) {
      scroll.firstLine = inOutFirstLine;
      renderScrollablePage(watchy, app, backLabel, upLabel, acceptLabel, downLabel);
      continue;
    }

    if (ev == ControlEvent::None) {
      delay(10);
    }
  }
}

void renderStatusLines(Watchy &watchy,
                       const char **lines,
                       uint8_t lineCount,
                       int16_t x,
                       int16_t y,
                       int16_t lineSpacing,
                       const GFXfont *font,
                       const char *backLabel) {
  UITextSpec specs[6]{};
  uint8_t count = 0;
  for (uint8_t i = 0; i < lineCount && i < 6; ++i) {
    if (lines[i] == nullptr) {
      continue;
    }
    specs[count].x = x;
    specs[count].y = y;
    specs[count].w = 0;
    specs[count].h = 0;
    specs[count].font = font;
    specs[count].fillBackground = false;
    specs[count].invert = false;
    specs[count].text = lines[i];
    ++count;
    y += lineSpacing;
  }

  UIAppSpec app{};
  app.texts = specs;
  app.textCount = count;

  setFixedRowControls(app, backLabel, "", "", "");
  UiSDK::renderApp(watchy, app);
}

static bool defaultButtonPressed(uint8_t pin) {
  return UiSDK::buttonPressed(pin);
}

bool runConfirmDialog2(Watchy &watchy,
                       const char *title,
                       const char *line1,
                       const char *line2,
                       const char *option0,
                       const char *option1,
                       const char *backLabel,
                       const char *upLabel,
                       const char *acceptLabel,
                       const char *downLabel,
                       const ConfirmDialogLayout &layout,
                       ButtonPressedFn buttonPressedFn) {
  int8_t selected = 0;

  (void)buttonPressedFn;
  beginControlCapture();
  clearControlEvent();

  UIControlsRowLayout controls[4] = {
      {backLabel, &Watchy::backPressed},
      {upLabel, &Watchy::upPressed},
      {acceptLabel, &Watchy::menuPressed},
      {downLabel, &Watchy::downPressed},
  };

  while (true) {
    UiSDK::initScreen(watchy.display);

    // Control labels (exact coordinates are caller-controlled).
    watchy.display.setCursor(layout.backX, layout.backY);
    watchy.display.print(backLabel ? backLabel : "BACK");
    watchy.display.setCursor(layout.upX, layout.upY);
    watchy.display.print(upLabel ? upLabel : "UP");
    watchy.display.setCursor(layout.acceptX, layout.acceptY);
    watchy.display.print(acceptLabel ? acceptLabel : "ACCEPT");
    watchy.display.setCursor(layout.downX, layout.downY);
    watchy.display.print(downLabel ? downLabel : "DOWN");

    // Title + optional lines.
    watchy.display.setCursor(layout.titleX, layout.titleY);
    watchy.display.print(title ? title : "Confirm");

    if (line1 && line1[0] != '\0') {
      watchy.display.setCursor(layout.line1X, layout.line1Y);
      watchy.display.print(line1);
    }
    if (line2 && line2[0] != '\0') {
      watchy.display.setCursor(layout.line2X, layout.line2Y);
      watchy.display.print(line2);
    }

    // Options.
    watchy.display.setCursor(layout.option1X, layout.option1Y);
    watchy.display.print(selected == 0 ? "> " : "  ");
    watchy.display.print(option0 ? option0 : "OK");

    watchy.display.setCursor(layout.option2X, layout.option2Y);
    watchy.display.print(selected == 1 ? "> " : "  ");
    watchy.display.print(option1 ? option1 : "Cancel");

    watchy.display.display(true);

    while (true) {
      UiSDK::renderControlsRow(watchy, controls);
      const ControlEvent ev = takeControlEvent();
      if (ev == ControlEvent::None) {
        delay(10);
        continue;
      }

      if (ev == ControlEvent::Back) {
        endControlCapture();
        return false;
      }

      if (ev == ControlEvent::Up) {
        selected = (selected == 0) ? 1 : 0;
        break;
      }

      if (ev == ControlEvent::Down) {
        selected = (selected == 1) ? 0 : 1;
        break;
      }

      if (ev == ControlEvent::Accept) {
        endControlCapture();
        return (selected == 0);
      }
    }
  }
}

} // namespace UiTemplates
