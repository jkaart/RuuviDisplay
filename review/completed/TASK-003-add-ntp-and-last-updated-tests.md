# Task ID: TASK-003

## Title: Add unit tests for NTP synchronization and "Last updated" row rendering

### Summary
The new `syncNtp()` function and `draw_last_updated_row()` introduce time-dependent behavior that should be tested. This task adds unit tests covering:
- NTP epoch handling (success, failure with 0 epoch).
- UTC-to-local timezone conversion via `utcToLocal()` wrapper.
- Display rendering of the "Last updated" row with various inputs.

### Context
New code paths introduced in this feature:
1. `syncNtp()` in `src/main.cpp` — syncs NTP and stores the epoch.
2. `draw_last_updated_row(uint8_t*)` in `src/display.cpp` — formats and draws the time string (or "--").

These are not trivial to test on hardware alone: the e-paper framebuffer is hardware-specific and NTP responses depend on network/server timing. Unit tests should exercise the logic that can be tested without the ESP32 runtime.

### Proposed Test Structure

**test/test_time_format.cpp (primary focus — pure C++17, no ESP32 deps)**
Tests the time formatting logic used by `draw_last_updated_row()` without hardware:
- Epoch 0 produces "--".
- Valid epochs produce correctly formatted strings.
- Unknown zone names cause a fallback to default timezone or "--".

**test/test_ntp_sync.cpp (native-test environment)**
Validates `syncNtp()` logic in isolation from hardware:
- `g_renderEpoch` is updated on success.
- Left unchanged on failure (idempotent).
- Handles rate limiting (interval between updates).

**test/test_display_last_updated_row.cpp (native-test environment)**
Requires the e-paper framebuffer types and `epd_highlevel.h`. A mock framebuffer captures draw commands for verification. If mocking a full framebuffer proves impractical, document that and rely on `test_time_format.cpp` for CI while leaving integration testing to hardware scripts.

### Acceptance Criteria
- Tests compile with a standard C++17 compiler without ESP-IDF headers.
- Edge cases covered: epoch=0 → "--", unknown zone → fallback, DST transitions.
- Build succeeds under PlatformIO's native-test environment.
- No existing functionality is broken.
