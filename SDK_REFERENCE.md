# Watchy SDK Reference (src/sdk)

This document is the single consolidated reference for everything under `src/sdk/**`:

- `UiSDK.h/.cpp`
- `UiTemplates.h/.cpp`
- `NetUtils.h/.cpp`
- `WatchfaceRegistry.h/.cpp`
- `Fonts.h`
- `UI_TEMPLATES.md` (legacy notes)

It focuses on:

- API/class/struct inventory
- how to use each module
- control-row embedding patterns
- input behavior and event flow
- practical conventions to keep app/watchface code clean

---

## 1) SDK at a glance

### Layering

1. **UiSDK**: low-level drawing + spec-based rendering + theme/polarity + debounced button helper.
2. **UiTemplates**: higher-level reusable screen loops (menus, viewers, dialogs, history pickers).
3. **NetUtils**: networking and text-input helpers used by internet apps.
4. **WatchfaceRegistry**: global watchface dispatch table + selector menu labels.
5. **Fonts**: bundled Adafruit GFX fonts + tiny bitmap fallback fonts.

### Display type used everywhere

`WatchyGxDisplay` is an alias of `ThemeableGxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT>`.

All render APIs use this display type directly or through `Watchy &watchy`.

---

## 2) UiSDK reference

Source: `src/sdk/UiSDK.h`, `src/sdk/UiSDK.cpp`

## 2.1 Core enums/types

### `enum class WatchfacePolarity`

- `WhiteOnBlack`
- `BlackOnWhite`

This is the authored polarity of a face/asset. Runtime theme is derived from global `gDarkMode` through `BASE_POLARITY`.

### `enum class UIElementKind`

- `Text`, `Button`, `Menu`, `Image`, `Media`

Type categorization helper; rendering is done via concrete specs.

### `struct Palette`

- `fg`, `bg`, `accentFg`, `accentBg`

Computed from polarity. Accent colors are used for selected/highlighted rows.

### `struct UIControlsRowLayout`

- `const char *label`
- `void (Watchy::*callback)()`

Represents one control slot (BACK/UP/ACCEPT/DOWN).

## 2.2 Declarative UI spec structs

### `UITextSpec`

- position/size: `x`, `y`, optional `w`, `h`
- style: `font`, `fillBackground`, `invert`
- content: `String text`

### `UIImageSpec`

- `bitmap`, `x`, `y`, `w`, `h`
- `fromProgmem`, `fillBackground`
- `polarityAuthored`

### `UIMenuItemSpec`

- `String label`

### `UIMenuSpec`

- geometry: `x`, `y`, `w`, `h`, `itemHeight`
- data: `items`, `itemCount`
- selection/scroll: `selectedIndex`, `startIndex`, `visibleCount`

### `UIButtonSpec`

- `x`, `y`, `w`, `h`, `font`, `label`, `selected`

### `UICheckboxSpec`

- `x`, `y`, `font`, `fillBackground`, `label`, `checked`

### `UICallbackSpec`

- `void (*draw)(WatchyGxDisplay&, void*)`
- `void *userData`

Use for custom render blocks that do not fit stock spec primitives.

### `UIScrollableTextSpec`

- geometry: `x`, `y`, `w`, `h`
- text source: `text` or external `textRef`
- viewport: `firstLine`, `maxLines`, `lineHeight`
- layout: `centered`, `wrapLongLines`, `wrapContinuationAlignRight`

### `UIAppChromeSpec`

- `beginFullScreen`, `drawControlsRow`, labels, `darkBorder`

Note: currently `renderApp()` always runs screen init + control-row flow regardless of these toggles; keep this struct as compatibility/future chrome metadata.

### `UIAppSpec`

Aggregates all element arrays + counts:

- `texts`, `images`, `menus`, `buttons`, `checkboxes`, `scrollTexts`, `callbacks`
- `controls[4]`, `controlCount`
- optional `chrome`

## 2.3 Theme and polarity APIs

- `setPolarity()`, `getPolarity()`
- `getThemePolarity()`, `liveThemePolarity()`
- `polarityForFaceId(uint8_t id)`
- `watchfaceUsesThemeColors(uint8_t id)`
- `getWatchfaceBg()`, `getWatchfaceFg()`

Typical watchface usage:

1. Determine authored polarity from face id or explicit value.
2. Set polarity before drawing assets.
3. Draw using SDK colors (`getWatchfaceFg/Bg`) where possible.

## 2.4 Font registry APIs

- `defaultFont()` (slot 0)
- `tinyFont5x7()`, `tinyFont6x8()`, `tinyMono6x8()`
- `registerFont(slot, font)`
- `getFont(slot)`

Registry has 64 slots. Tiny fonts are recognized by sentinel pointers and rendered by custom bitmap glyph path.

## 2.5 Rendering functions

- `renderText`, `renderImage`, `renderMenu`, `renderButton`, `renderCheckbox`
- `renderScrollableText`, `renderCallback`
- `countScrollableTextLines`
- `drawTextLine`

### Render order inside full app render

`renderApp()` paints in this order:

1. `initScreen()`
2. control-row handling (`renderControlsRow`)
3. callbacks
4. images
5. menus
6. buttons
7. checkboxes
8. scrollable texts
9. texts
10. `display(true)`

`renderWatchfaceSpec()` uses a similar content order but without control-row dispatch.

## 2.6 Input helper

`buttonPressed(pin, debounceMs=80, activeLow=true)`:

- per-pin debounce slots
- edge-latched behavior (fires once per press until release)
- defaults tuned for menu responsiveness

## 2.7 Display wrapper methods

`UiSDK` exposes inline wrappers for core GxEPD/GFX methods (`init`, `setFullWindow`, `fillScreen`, `drawBitmap`, text setters, bounds, etc.) so app/watchface code can stay namespace-consistent.

---

## 3) UiTemplates reference

Source: `src/sdk/UiTemplates.h`, `src/sdk/UiTemplates.cpp`

`UiTemplates` provides reusable loops/layouts for common app UX patterns.

## 3.1 Control capture/event model

### `enum class ControlEvent`

- `None`, `Back`, `Up`, `Accept`, `Down`

### Capture APIs

- `beginControlCapture()` / `endControlCapture()`
- `isCapturingControls()`
- `postControlEvent()`, `takeControlEvent()`, `clearControlEvent()`

Use capture mode when running blocking template loops so global button callbacks post events instead of directly invoking app actions.

### Press carry-over guard

`waitForAllButtonsReleased(stableMs=40, timeoutMs=600)` prevents accidental immediate trigger when transitioning screens.

## 3.2 Layout structs

- `ControlsRowLayout`: custom coordinate placement for labels.
- `MenuPickerLayout`: header/menu positions, row count, optional auto-scroll.
- `ConfirmDialogLayout`: complete 2-option dialog geometry.
- `ToastLayout`: small two-option floating prompt geometry.
- `HistoryPickerSpec`: NVS-backed history menu behavior and labels.

## 3.3 Menu helpers

- `calcMenuStartIndex(selected, visibleRows, itemCount)`
- `keepMenuSelectionVisible(selected, visibleRows, itemCount, inOutStartIndex)`

Use the second helper when you want “scroll inertia” (adjust only when selection leaves viewport).

## 3.4 Page renderers

- `renderBarePage()`
- `renderPageWithControlsAt()`
- `renderMenuPickerPage()`
- `renderScrollablePage()` / `renderPage()` alias
- `renderStatusLines()`
- `renderControlsRowAt()`
- `renderCenteredMonospaceHighlight()`

## 3.5 Blocking template loops

### `runMenuPicker(...)`

- BACK returns `-1`
- UP/DOWN cycles selection with wrap
- ACCEPT returns selected index and updates `inOutSelectedIndex`

### `runScrollableViewer(...)`

- BACK exits (`ViewerAction::Back`)
- UP/DOWN scroll
- optional ACCEPT exit (`ViewerAction::Accept`) when `acceptEnabled=true`
- supports hold-repeat scrolling (very short repeat interval)

### `runRefreshableScrollableViewer(...)`

- same scrolling behavior
- ACCEPT triggers rebuild callback (`BuildScrollableTextFn`)

### `runConfirmDialog2(...)`

- two-option selector
- BACK = cancel/false
- ACCEPT returns true only if option0 selected

### `runToast2Option(...)`

- BACK returns `-1`
- ACCEPT returns chosen option index (`0` or `1`)

## 3.6 NVS history picker flow

### `runHistoryPickerNvs(...)`

Provides:

- list of existing history entries
- optional example quick-pick
- “new target” row
- BACK toast with delete-current or exit-app choices

Outputs:

- selected value (`outValue`)
- `HistoryPickKind` (`HistoryItem` / `ExampleQuickPick` / `NewTarget`)

### `runHistoryPickerEditAndPersistNvs(...)`

Pipeline:

1. pick from history
2. edit value via `NetUtils::editTarget()`
3. persist only when non-empty and changed

### `historyAddUniqueNvs(...)`

Adds unique value to history; drops oldest when max entries reached.

---

## 4) NetUtils reference

Source: `src/sdk/NetUtils.h`, `src/sdk/NetUtils.cpp`

## 4.1 Data struct

### `struct NetResponse`

- `httpCode`
- `body`
- `error`
- `location`
- `contentType`
- `contentEncoding`

## 4.2 Wi-Fi/session helper

`ensureWiFi(watchy, retries, attemptTimeoutMs)`:

- delegates to `watchy.connectWiFi()`
- retries connection attempts
- waits for `WL_CONNECTED`

## 4.3 HTTP helpers

### `httpGet(url, timeoutMs, shutdownRadio)`

- follows redirects manually (bounded)
- supports HTTPS with insecure TLS client
- forces `Accept-Encoding: identity` (avoid unsupported compression decoding)
- reads response stream incrementally with memory cap
- keeps head + rolling tail for oversized payloads
- can disable Wi-Fi/BT after call (`shutdownRadio=true`)

### `httpPostForm(url, formBody, origin, referer, acceptLanguage, timeoutMs, shutdownRadio)`

Same resilience behavior as GET + form URL-encoded POST body.

## 4.4 Parsing/lookup/network tools

- `parseIp(ip, out[4])`
- `whoisLookup(target, timeoutMs, maxBytes)` (IANA + referral hop)
- `dnsLookup(name, qtype, timeoutMs, attempts)` (minimal UDP DNS client; auto PTR for IPv4)
- `traceroute(host, maxHops, timeoutMs)` (ICMP echo TTL probing)

## 4.5 Uniform target editor

### `editTarget(watchy, buffer, maxLen, title, defaultValue)`

Behavior:

- allowed chars: space, `a-z`, `0-9`, `-`, `.`
- UP/DOWN: change current character
- ACCEPT: append char
- BACK: delete char; if empty => cancel
- double ACCEPT on trailing space => finish

State helpers:

- `consumeExitToMenuRequest()` consumes long-press/exit request flag.

---

## 5) WatchfaceRegistry reference

Source: `src/sdk/WatchfaceRegistry.h`, `src/sdk/WatchfaceRegistry.cpp`

Namespace: `WatchfaceRegistry`

### `struct Entry`

- `const char *name`
- `void (*draw)(Watchy &watchy)`

### Globals

- `kWatchfaceCount`
- `kWatchfaceMenuItems[]`
- `kWatchfaces[]`

Purpose:

- single source of truth for watchface selector menu labels and draw dispatch function pointers.
- order is meaningful (selector uses table order).

---

## 6) Fonts reference

Source: `src/sdk/Fonts.h`

Contains:

- broad Adafruit GFX font includes (Mono/Sans/Serif families and variants)
- tiny bitmap fallback arrays:
  - `sTinyFont5x7`
  - `sTinyFont6x8`
  - `sTinyMonospaceFont6x8`

`UiSDK` uses sentinel pointers to route tiny fonts through a lightweight bitmap renderer.

---

## 7) Controls embedding patterns

## 7.1 Standard app row (recommended)

Populate `UIAppSpec.controls[4]` with `BACK/UP/ACCEPT/DOWN` labels + callbacks and call `UiSDK::renderApp()`.

Example:

```cpp
UIAppSpec app{};
app.controls[0] = {"BACK", &Watchy::backPressed};
app.controls[1] = {"UP", &Watchy::upPressed};
app.controls[2] = {"ACCEPT", &Watchy::menuPressed};
app.controls[3] = {"DOWN", &Watchy::downPressed};
UiSDK::renderApp(watchy, app);
```

## 7.2 Coordinate-specific controls row

When a screen must match a legacy pixel layout, draw labels with `UiTemplates::renderControlsRowAt(...)` and keep input loop separate.

## 7.3 Template-owned input loops

For menu/viewer/dialog screens, prefer `UiTemplates::run*` APIs. They:

- establish control capture
- guard against carried-over presses
- own event polling and return high-level actions

---

## 8) Input behavior details

## 8.1 Electrical/read assumptions

- buttons are active-low (`digitalRead(pin) == ACTIVE_LOW`)
- debounce in `UiSDK::buttonPressed` is edge-latched

## 8.2 Repeat behavior

- generic control rows: debounced press events
- scroll viewers: additional raw held-state sampling for fast UP/DOWN repeat

## 8.3 Event ownership

- if using `UiTemplates` blocking loops, always pair `beginControlCapture()` with `endControlCapture()`.
- always call `waitForAllButtonsReleased()` when entering a modal/loop from another interactive screen.

---

## 9) Keeping code neat (project conventions)

1. **Use spec structs, not ad-hoc draw code**, unless truly custom.
2. **Keep rendering declarative**: build `UIAppSpec` near top of draw function, then render once.
3. **Separate input from painting**:
   - rendering helpers are render-only
   - template loops own input
4. **Reuse templates** for repeated families (menu, scroll viewer, confirm, history picker).
5. **Use `textRef` for large text payloads** to avoid unnecessary `String` copies.
6. **Respect polarity/theme APIs**; avoid hardcoding black/white where theme adaptation is expected.
7. **Cap buffers and list sizes** with provided constants (`HISTORY_MAX_ENTRIES`, `HISTORY_MAX_ENTRY_LEN`).
8. **Persist selection state in/out parameters** (`inOutSelectedIndex`, `inOutFirstLine`) for good UX continuity.

---

## 10) Practical usage recipes

## 10.1 Minimal static page

```cpp
UITextSpec title{.x=10, .y=36, .font=UiSDK::defaultFont(), .text="My App"};
UIAppSpec app{};
app.texts = &title;
app.textCount = 1;
app.controls[0] = {"BACK", &Watchy::backPressed};
UiSDK::renderApp(watchy, app);
```

## 10.2 Menu picker page

```cpp
int8_t selected = 0;
int8_t result = UiTemplates::runMenuPicker(
  watchy,
  "Choose",
  items,
  itemCount,
  selected
);
```

## 10.3 Scrollable results viewer

```cpp
uint16_t firstLine = 0;
UiTemplates::ViewerAction action = UiTemplates::runScrollableViewer(
  watchy,
  app,
  scrollSpec,
  firstLine,
  "BACK", "UP", "ACCEPT", "DOWN",
  true
);
```

## 10.4 Network + edit + persist flow

```cpp
int8_t selectedIdx = -1;
char target[128] = {};
UiTemplates::HistoryPickerSpec spec{};
spec.nvsNamespace = "ping_hist";

if (UiTemplates::runHistoryPickerEditAndPersistNvs(
      watchy, spec, target, sizeof(target), "Target", selectedIdx)) {
  // run request with target
}
```

---

## 11) Notes on `src/sdk/UI_TEMPLATES.md`

That file is historical design notes and can drift from implementation. Prefer `UiTemplates.h/.cpp` as source of truth; this root `SDK_REFERENCE.md` is aligned to current code.
