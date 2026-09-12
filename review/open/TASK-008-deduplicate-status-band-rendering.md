# Task ID: TASK-008

status: open

## Title: Deduplicate the status-band rendering sequence in display.cpp

### Summary
`src/display.cpp` repeats the same three-step "status band" sequence in both
`display_update()` and `display_show_error()`: erase the band, draw the "Last updated" row,
then draw the local battery voltage row. This duplication was introduced/extended by the
`feat/lilygo-battery-stats` branch when it added `draw_battery_voltage_row()` in both
places. Extract the shared sequence into one helper so future changes to the band (layout,
content, erase geometry) are made in a single place.

### Context

Verified in `src/display.cpp`:

- `display_update()` (lines ~130-141):
  ```
  epd_fill_rect(EpdRect{...LAST_UPDATED_ROW_Y...}, 0xFF, g_fb);   // erase status band
  draw_last_updated_row(g_fb);
  draw_battery_voltage_row(g_fb);
  ```
- `display_show_error()` (lines ~222-230): identical erase rectangle, then
  `draw_last_updated_row(g_fb);` and `draw_battery_voltage_row(g_fb);`.

The erase rectangle (`EpdRect{.x = 0, .y = LAST_UPDATED_ROW_Y, .width = DISPLAY_WIDTH,
.height = DISPLAY_HEIGHT - LAST_UPDATED_ROW_Y}`) is byte-for-byte identical in both call
sites. Any future change affecting the status band (for example moving the battery row,
changing the band height, or adding another status element) currently requires touching two
functions and risks divergence.

### Proposed Changes

**`src/display.cpp`**:

- Add a static helper, e.g.:
  ```
  // Erase the status band and redraw the "Last updated" row + local 18650 cell voltage.
  static void draw_status_band(uint8_t *fb);
  ```
  The helper performs the `epd_fill_rect` erase, `draw_last_updated_row(fb)` and
  `draw_battery_voltage_row(fb)`.
- Replace the duplicated sequence in `display_update()` and `display_show_error()` with a
  single call to the helper.
- Keep the existing forward declaration of `draw_battery_voltage_row` (or fold it into the
  new helper) so the helper can be defined in the same existing layout order.

No other rendering behavior changes.

### Acceptance Criteria
- The erase + row-draw sequence appears exactly once in `src/display.cpp`.
- `display_update()` and `display_show_error()` each call the shared helper.
- Firmware build succeeds: `pio run -e lilygo-t5-47`.
- Visual output is unchanged (same framebuffer content on both the OK and error paths).