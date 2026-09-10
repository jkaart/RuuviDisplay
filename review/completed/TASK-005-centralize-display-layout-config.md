# Task ID: TASK-005

status: completed

## Title: Centralize display layout constants into a dedicated configuration header

### Summary
`src/display.cpp` contains multiple magic-number constants that define the visual layout of the e-paper display. These values are currently scattered as local static variables with no centralized definition, making them hard to adjust and error-prone to maintain. This task consolidates all layout-related constants into a dedicated header (`include/display_layout.h`), improving readability and enabling future adjustments in one place.

### Context
The following "magic numbers" are currently defined directly in `src/display.cpp`:

| Constant | Value | Purpose |
|----------|-------|---------|
| `ERROR_BAND_Y_GAP` | 4 | vertical gap between error band and bottom edge |
| `LAST_UPDATED_GAP` | 18 | gap between "Last updated" row and the error line |
| `TS_Y` | 448 | Y-coordinate for tag timestamps (from top of screen) |
| `ICON_WIDTH` / `ICON_HEIGHT` | 60 | icon square size |
| `GAP` | 5 | gap between icon and value+text |
| `HW` | 160 | half panel width -> panels are exactly 320 wide |
| `SCREEN_W` / `SCREEN_H` | 960 / 540 | landscape framebuffer dimensions |
| `PANEL_CX` | {160, 480, 800} | left edge of each panel |

Changing any of these would require careful manual updates across several lines — a recipe for subtle visual regressions.

### Proposed Changes

**New file: `include/display_layout.h`**
```cpp
#pragma once

#include <cstdint>

namespace display_layout {

constexpr uint8_t DISPLAY_WIDTH  = 960;
constexpr uint8_t DISPLAY_HEIGHT = 540;
constexpr uint8_t PANEL_COUNT    = 3;
constexpr int16_t PANEL_HALF_WIDTH = DISPLAY_WIDTH / 2;  // 480
constexpr int16_t PANEL_CX[3] = {160, 480, 800};

constexpr uint8_t ICON_WIDTH    = 60;
constexpr uint8_t ICON_HEIGHT   = ICON_WIDTH;

constexpr int16_t ICON_MARGIN_X   = 5;
constexpr int16_t GAP_ICON_TEXT   = 5;
constexpr uint8_t TS_Y_OFFSET     = 448;

constexpr int16_t ERROR_BAND_Y_GAP       = 4;
constexpr int16_t LAST_UPDATED_GAP       = 18;
constexpr int16_t ERROR_BAND_Y           = DISPLAY_HEIGHT - ERROR_BAND_Y_GAP;    // 536
constexpr int16_t LAST_UPDATED_ROW_Y     = ERROR_BAND_Y - LAST_UPDATED_GAP;      // 518

} // namespace display_layout
```

**Modify `src/display.cpp`:** Replace all occurrences of the old constants with references to the new header.

### Acceptance Criteria
- All magic numbers in `src/display.cpp` are replaced with references to `display_layout`.
- Build succeeds without modification.
- Visual output is identical (no regression).
- A simple change to, e.g., `LAST_UPDATED_GAP` can be made in one file and the effect is immediately visible across all uses.

## Resolution

### What was changed
- **New file `include/display_layout.h`**: consolidated all layout constants into a
  `display_layout` namespace (screen/panel geometry, icon size, margins/gaps, timestamp
  Y and the derived status-band/"Last updated" row Y positions).
- **`src/display.cpp`**: removed the 13 scattered `static const`/`#define` definitions and
  replaced every usage with a `display_layout::` reference.

### Deviations from the proposed header (necessary to satisfy "no visual regression")
The task's proposed header contained three bugs that would have changed the rendered
pixels. Per AGENTS.md ("Do not blindly trust the task description") and the hard
acceptance criterion "Visual output is identical (no regression)", each was corrected:

1. `PANEL_HALF_WIDTH` — proposed `DISPLAY_WIDTH / 2` (= 480) would have pushed icons off
   screen and text into the next panel. Original `HW` was 160 (320-wide panels). Kept at
   **160**.
2. `ERROR_BAND_Y` — proposed `DISPLAY_HEIGHT - 4` (= 536) moved the error line down by 10.
   Original was `530 - 4` = 526. Kept at **526** (this also keeps `LAST_UPDATED_ROW_Y` at
   508).
3. Types — proposed `uint8_t` for `DISPLAY_WIDTH` (960), `DISPLAY_HEIGHT` (540) and
   `TS_Y_OFFSET` (448) would truncate values above 255 (e.g. 960->192, 448->192), moving
   every coordinate. Changed those three to **`int16_t`**.

All other constant names/values/types from the proposal were kept as-is.

### Tests / verification
- Firmware environment `lilygo-t5-47` builds cleanly (no errors/warnings), both incrementally
  and from a clean build.
- Standalone C++ check including the header asserts all 16 constants and 4 derived
  positions (icon_x, text_x, fill width/height) equal the original values — 20/20 pass.
- `pio test` (native `test` env) unchanged: it has no suites (pre-existing), unrelated to this change.

### Acceptance criteria
- [x] All magic numbers in `src/display.cpp` replaced with `display_layout::` references.
- [x] Build succeeds without modification.
- [x] Visual output is identical (no regression) — all original pixel positions preserved.
- [x] `LAST_UPDATED_GAP` is a single header constant; changing it immediately affects both
      the status-band erase height and the "Last updated" row position.
