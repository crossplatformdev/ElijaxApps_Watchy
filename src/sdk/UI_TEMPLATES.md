# UI Templates Catalog

This firmware has several repeated UI "families" (layouts + interaction loops). The goal of these templates is to keep a consistent look & feel, reduce copy/paste boilerplate, and make future apps easier to implement **without changing appearance or behavior**.

## Families

### 1) `MenuPicker` (Header + List + Controls Row)
**Used for:** history pickers (Dig/WhoIS/Traceroute/Port Scan), type pickers (Dig qtype), preset pickers (Postman).

**Layout:**
- Header: `y=36`, bold 9pt font
- Menu list: `y=72`, 4 visible rows (or less)
- Controls: `BACK / UP / ACCEPT / DOWN`

**Template API:** `UiTemplates::runMenuPicker(...)`

### 2) `ScrollablePage` (Scrollable text + Controls Row)
**Used for:** network tool results and other text-heavy views.

**Template API:**
- `UiTemplates::renderScrollablePage(...)` (render-only)
- `UiTemplates::runScrollableViewer(...)` (blocking viewer loop: BACK exits, UP/DOWN scroll, optional MENU accept)

**Related API:**
- `UiTemplates::renderPage(...)` is an alias of `renderScrollablePage(...)` for non-scroll pages that still follow the same "full redraw + controls row" convention.

**Notes / invariants:**
- Templates clear and fill the full screen each frame to avoid ghosting artifacts.
- Templates must not override caller geometry (x/y/w/h/fonts) and must not change the app's control labels.

### 3) `RefreshableScrollableViewer` (Scrollable page with MENU refresh)
**Used for:** astronomy apps that support `MENU: refresh` (SunRise/MoonRise/MoonPhase).

**Behavior:**
- BACK exits
- UP/DOWN scroll
- MENU rebuilds the text using a callback and rerenders

**Template API:** `UiTemplates::runRefreshableScrollableViewer(...)`

**Notes / invariants:**
- `scroll.maxLines` is treated as caller-owned; it is only auto-computed when `maxLines==0`.
- Text is rebuilt into a caller-provided scratch `String` and rendered via `scroll.textRef`.

### 4) `ConfirmDialog2` (Two-option confirm)
**Used for:** destructive confirmations (e.g. Ping delete/stop confirmations).

**Behavior:**
- BACK returns `false` (cancel)
- UP/DOWN toggles selection
- MENU accepts; returns `true` only when option 0 is selected

**Template API:** `UiTemplates::runConfirmDialog2(...)`

**Notes / invariants:**
- Layout is fully coordinate-controlled via `ConfirmDialogLayout` to preserve pixel-identical screens.
- Input handling can use the default debounced `UiSDK::buttonPressed`, or a caller-provided `ButtonPressedFn` when an app has custom debounce semantics.

### 5) `StatusLines` (Simple Status Screen)
**Used for:** setup/OTA screens that show a few lines of status and a BACK hint.

**Template API:** `UiTemplates::renderStatusLines(...)`

### 6) `BarePage` (App render-only)
**Used for:** one-shot OS screens that are purely informational and do not show a controls row (About, Theme info, Buzz).

**Template API:** `UiTemplates::renderBarePage(...)`

### 7) `PageWithControlsAt` (App + bespoke controls coordinates)
**Used for:** Ping-style screens that must keep control labels at exact coordinates (top row + bottom row).

**Template API:** `UiTemplates::renderPageWithControlsAt(...)` + `renderControlsRowAt(...)`

**Notes / invariants:**
- Template intentionally does not change the display font; callers that rely on a specific font for control labels must set it before calling.

### 8) `DigitEditor` (Centered monospace + highlighted digit)
**Used for:** Ping IP digit-by-digit editor.

**Building blocks:**
- `UiTemplates::renderCenteredMonospaceHighlight(...)` for the centered string + per-character highlight.
- `UiTemplates::renderControlsRowAt(...)` when the screen uses bespoke control-label coordinates.

**Notes / invariants:**
- Assumes a fixed-width font (monospace) so centering is stable and the highlight box aligns with the selected character.

## Helpers

### Menu scrolling helpers
Used to keep menu selection visible while preserving app-specific scrolling semantics.

- `UiTemplates::calcMenuStartIndex(...)` computes a start index that keeps the selection visible.
- `UiTemplates::keepMenuSelectionVisible(...)` adjusts an existing start index only when needed (preserves "scroll inertia").

### Render-only helpers
Used when an app must keep bespoke input semantics (custom debounce, special BACK handling, etc.) but can share the standard painting.

- `UiTemplates::renderMenuPickerPage(...)` renders the MenuPicker layout without running an input loop.

### Bespoke layout primitives
Used for screens that intentionally do not match the standard controls-row coordinates.

- `UiTemplates::renderControlsRowAt(...)` renders labels at caller-provided coordinates (`ControlsRowLayout`).
- `UiTemplates::renderCenteredMonospaceHighlight(...)` draws a centered fixed-width string and inverts one character index (useful for digit editors like Ping).

### Screen setup helper
- `UiTemplates::beginFullScreen(...)` performs the common clear + fill + textColor setup used by many bespoke screens.

## Wrapping
The underlying scroll renderer (`UiSDK::renderScrollableText`) supports opt-in wrapping:
- `UIScrollableTextSpec::wrapLongLines=true` splits long lines into multiple visual lines.
- `wrapContinuationAlignRight=true` renders continuation segments aligned to the right.

Counting visual lines is handled by `UiSDK::countScrollableTextLines(...)`, which apps should use when wrapping is enabled.
