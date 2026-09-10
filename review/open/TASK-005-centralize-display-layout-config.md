# Task ID: TASK-005

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
