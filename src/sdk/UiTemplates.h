#ifndef UI_TEMPLATES_H
#define UI_TEMPLATES_H

#include "UiSDK.h"

// Higher-level interactive UI templates built on top of UiSDK.
// These are intended to reduce repeated app boilerplate while keeping
// appearance/behavior consistent across "families" of screens.

namespace UiTemplates {

enum class ControlEvent : uint8_t {
  None,
  Back,
  Up,
  Accept,
  Down,
};

// Posted by Watchy::backPressed/menuPressed/upPressed/downPressed while
// UiTemplates is capturing controls.
void postControlEvent(ControlEvent ev);
ControlEvent takeControlEvent();
void clearControlEvent();

// When enabled, the four global Watchy callbacks post ControlEvent instead of
// being dispatched to the active app.
void beginControlCapture();
void endControlCapture();
bool isCapturingControls();

// Guard helper: waits until all four buttons are released for a stable period.
// Useful when entering an app to avoid a carried-over press immediately firing
// the new screen's handlers.
void waitForAllButtonsReleased(uint32_t stableMs = 40, uint32_t timeoutMs = 600);

// --- History storage limits ---
// Required invariants for all apps using SDK-backed history.
static constexpr uint8_t HISTORY_MAX_ENTRIES = 42;
static constexpr uint16_t HISTORY_MAX_ENTRY_LEN = 255;

using ButtonPressedFn = bool (*)(uint8_t pin);

struct ControlsRowLayout {
  // Default coordinates match Ping's bespoke screens.
  int16_t backX = 8;
  int16_t backY = 16;
  int16_t upX = 164;
  int16_t upY = 16;
  int16_t acceptX = 8;
  int16_t acceptY = 192;
  int16_t downX = 152;
  int16_t downY = 192;
};

// Renders control labels at exact coordinates (useful for bespoke screens that
// cannot use UiSDK::renderControlsRow's fixed layout).
void renderControlsRowAt(WatchyGxDisplay &display,
                         const char *backLabel,
                         const char *upLabel,
                         const char *acceptLabel,
                         const char *downLabel,
                         const ControlsRowLayout &layout = ControlsRowLayout{});

// Render-only page without a controls row.
void renderBarePage(Watchy &watchy, const UIAppSpec &app);

// Render-only page with a coordinate-controlled controls row.
void renderPageWithControlsAt(Watchy &watchy,
                              const UIAppSpec &app,
                              const char *backLabel,
                              const char *upLabel,
                              const char *acceptLabel,
                              const char *downLabel,
                              const ControlsRowLayout &controlsLayout = ControlsRowLayout{});

struct MenuPickerLayout {
  int16_t headerX = 16;
  int16_t headerY = 36;
  int16_t menuX = 0;
  int16_t menuY = 72;
  uint8_t visibleRowsMax = 4;
  uint8_t startIndex = 0; // keep 0 for apps that intentionally don't scroll menus
  bool autoScroll = false; // if true, startIndex is auto-computed to keep selection visible
  const GFXfont *font = &FreeMonoBold9pt7b;
};

// Compute a menu startIndex so that selectedIndex stays visible within a
// window of size visibleRows.
uint8_t calcMenuStartIndex(uint8_t selectedIndex, uint8_t visibleRows, uint8_t itemCount);

// Adjust an existing startIndex only when needed to keep the selectedIndex
// within the visible window. This preserves the "scroll inertia" behavior
// some menus rely on.
void keepMenuSelectionVisible(uint8_t selectedIndex,
                              uint8_t visibleRows,
                              uint8_t itemCount,
                              uint8_t &inOutStartIndex);

// Runs a simple full-screen menu picker.
// Returns:
//  -1 if BACK was pressed
//  otherwise the selected index (0..itemCount-1)
int8_t runMenuPicker(Watchy &watchy,
                     const char *title,
                     const UIMenuItemSpec *items,
                     uint8_t itemCount,
                     int8_t &inOutSelectedIndex,
                     const MenuPickerLayout &layout = MenuPickerLayout{},
                     const char *backLabel = "BACK",
                     const char *upLabel = "UP",
                     const char *acceptLabel = "ACCEPT",
                     const char *downLabel = "DOWN");

// Render-only variant of MenuPicker. Useful when an app needs bespoke input
// semantics but wants to share the standard layout/painting.
void renderMenuPickerPage(Watchy &watchy,
                          const char *title,
                          const UIMenuItemSpec *items,
                          uint8_t itemCount,
                          int8_t selectedIndex,
                          const MenuPickerLayout &layout = MenuPickerLayout{},
                          const char *backLabel = "BACK",
                          const char *upLabel = "UP",
                          const char *acceptLabel = "ACCEPT",
                          const char *downLabel = "DOWN");

// Draw a fixed-width (monospace) string centered on screen and highlight one
// character index by inverting its background.
void renderCenteredMonospaceHighlight(WatchyGxDisplay &display,
                                     const GFXfont *font,
                                     const char *text,
                                     uint8_t textLen,
                                     int16_t y,
                                     int8_t highlightIndex,
                                     uint16_t fg,
                                     uint16_t bg,
                                     int16_t highlightExtraHeight = 2);

// Renders a standard scrollable-text page with the given control labels.
// Does NOT modify scroll.firstLine.
void renderScrollablePage(Watchy &watchy,
                          const UIAppSpec &app,
                          const char *backLabel,
                          const char *upLabel,
                          const char *acceptLabel,
                          const char *downLabel);

// Generic page renderer (alias of renderScrollablePage; name reflects that the
// page may or may not contain scrollable elements).
inline void renderPage(Watchy &watchy,
                       const UIAppSpec &app,
                       const char *backLabel,
                       const char *upLabel,
                       const char *acceptLabel,
                       const char *downLabel) {
  renderScrollablePage(watchy, app, backLabel, upLabel, acceptLabel, downLabel);
}

enum class ViewerAction : uint8_t {
  Back,
  Accept,
};

// Runs a generic scroll viewer loop:
// - BACK exits
// - UP/DOWN scroll
// - MENU returns ViewerAction::Accept (if acceptEnabled=true)
ViewerAction runScrollableViewer(Watchy &watchy,
                                 UIAppSpec &app,
                                 UIScrollableTextSpec &scroll,
                                 uint16_t &inOutFirstLine,
                                 const char *backLabel,
                                 const char *upLabel,
                                 const char *acceptLabel,
                                 const char *downLabel,
                                 bool acceptEnabled);

// Refreshable scroll-viewer loop:
// - Renders a scrollable text page
// - BACK exits
// - UP/DOWN scroll
// - MENU triggers a refresh (rebuilds the text using the callback)
using BuildScrollableTextFn = void (*)(Watchy &watchy, void *userData, String &outText);

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
                                           String &scratchText);

struct ConfirmDialogLayout {
  const GFXfont *font = &FreeMonoBold9pt7b;

  // Control labels positions (these default to Ping's confirm screens).
  int16_t backX = 8;
  int16_t backY = 16;
  int16_t upX = 164;
  int16_t upY = 16;
  int16_t acceptX = 8;
  int16_t acceptY = 192;
  int16_t downX = 152;
  int16_t downY = 192;

  // Content positions.
  int16_t titleX = 10;
  int16_t titleY = 36;
  int16_t line1X = 10;
  int16_t line1Y = 65;
  int16_t line2X = 10;
  int16_t line2Y = 85;
  int16_t option1X = 10;
  int16_t option1Y = 120;
  int16_t option2X = 10;
  int16_t option2Y = 145;
};

// Two-option confirm dialog:
// - BACK returns false
// - UP/DOWN toggles selection
// - MENU accepts selection; returns true only when option 0 is selected
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
                       ButtonPressedFn buttonPressedFn = nullptr);

// Simple status screen: renders up to `lineCount` lines of text using a fixed font
// and spacing. Useful for WiFi setup / OTA update flows.
void renderStatusLines(Watchy &watchy,
                       const char **lines,
                       uint8_t lineCount,
                       int16_t x,
                       int16_t y,
                       int16_t lineSpacing,
                       const GFXfont *font,
                       const char *backLabel = "BACK");

// --- Toast / small action prompt ---

struct ToastLayout {
  const GFXfont *font = &FreeMonoBold9pt7b;
  int16_t x = 10;
  int16_t y = 90;
  int16_t w = WatchyDisplay::WIDTH - 20;
  int16_t h = 90;
  int16_t titleX = 16;
  int16_t titleY = 112;
  int16_t option0X = 16;
  int16_t option0Y = 142;
  int16_t option1X = 16;
  int16_t option1Y = 166;
};

// Two-option toast-style picker.
// Returns:
//  -1 if BACK cancels
//  0 if option0 chosen
//  1 if option1 chosen
int8_t runToast2Option(Watchy &watchy,
                       const char *title,
                       const char *option0,
                       const char *option1,
                       int8_t defaultSelection = 1,
                       const ToastLayout &layout = ToastLayout{},
                       const char *backLabel = "BACK",
                       const char *upLabel = "UP",
                       const char *acceptLabel = "ACCEPT",
                       const char *downLabel = "DOWN");

// --- NVS-backed history picker (host/url style) ---

enum class HistoryPickKind : uint8_t {
  HistoryItem,
  ExampleQuickPick,
  NewTarget,
};

struct HistoryPickerSpec {
  const char *title = nullptr;
  const char *nvsNamespace = nullptr; // required
  // If exampleLabel is provided, it will be shown in the menu while exampleValue
  // is written into outValue when selected.
  const char *exampleLabel = nullptr;
  const char *exampleValue = "example.com";
  const char *newLabel = "New target";

  // Storage limits
  uint8_t maxEntries = HISTORY_MAX_ENTRIES;
  uint16_t maxEntryLen = HISTORY_MAX_ENTRY_LEN;

  // Menu appearance
  MenuPickerLayout menuLayout = MenuPickerLayout{};

  // BACK action prompt strings
  const char *backPromptTitle = "History";
  const char *deleteRecordLabel = "Delete record";
  const char *exitAppLabel = "Exit App";
};

// Interactive history picker with:
// - selectable history entries
// - optional example quick-pick (only shown when not already in history)
// - a "New target" entry
// - BACK -> toast: Delete record / Exit App
// Returns false only when user chooses Exit App.
bool runHistoryPickerNvs(Watchy &watchy,
                         const HistoryPickerSpec &spec,
                         char *outValue,
                         size_t outValueSize,
                         HistoryPickKind &outKind,
                         int8_t &inOutSelectedIndex);

// Convenience wrapper:
// 1) runs runHistoryPickerNvs()
// 2) opens NetUtils::editTarget() for user editing
// 3) persists to NVS only if the final value is non-empty AND different from the initially chosen value
// Returns false when the user exits the app from the history list OR cancels the editor.
// Returns true when a value is accepted (even if unchanged/not persisted).
// Note: if editTarget() returns false due to long-press BACK, callers can use
// NetUtils::consumeExitToMenuRequest() as usual.
bool runHistoryPickerEditAndPersistNvs(Watchy &watchy,
                                      const HistoryPickerSpec &spec,
                                      char *inOutValue,
                                      size_t inOutValueSize,
                                      const char *editorTitle,
                                      int8_t &inOutSelectedIndex);

// Add a value to the NVS history (unique; drops oldest when full).
void historyAddUniqueNvs(const char *nvsNamespace,
                         const char *value,
                         uint8_t maxEntries = HISTORY_MAX_ENTRIES,
                         uint16_t maxEntryLen = HISTORY_MAX_ENTRY_LEN);

} // namespace UiTemplates

#endif // UI_TEMPLATES_H
