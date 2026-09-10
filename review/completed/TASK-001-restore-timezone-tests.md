# Task ID: TASK-001

## Title: Restore timezone unit test coverage after deletion of test/test_timezone.cpp

### Summary
The commit that introduced the NTP-based "Last updated" row feature deleted `test/test_timezone.cpp`. That file held portable (host-side) unit tests for the timezone conversion logic, verifying:
- Each zone's stored UTC offset matches its real-world standard-time rule.
- DST rules correctly widen the offset or equal it for zones without DST.
- Unknown zone names return nullptr (no silent defaulting).

Deleting it removed the project's automated verification of the Timezone-library integration. This task restores equivalent tests in a location compatible with PlatformIO's native-test framework.

### Context
The file was removed in the current commit:
    Remove portable unit tests for timezone functionality from test/test_timezone.cpp

The deleted tests were valuable because they run on any host compiler without the Arduino/ESP32 runtime, making them suitable for CI and local development.

### Proposed Changes

**New file: `test/test_timezone_table.cpp`**
- Links against `src/timezone_table.cpp` to validate the built-in zone table.
- Asserts each known timezone has correct standard/DST offsets.
- Asserts unknown zones return nullptr.

**New file: `test/test_time_format.cpp` (optional, edge cases)**
- Epoch 0 produces "--".
- Valid epochs produce correctly formatted strings.
- Unknown zone names fall back to default timezone or "--".

### Acceptance Criteria
- The new test file compiles with a standard C++17 toolchain (no ESP-IDF/Arduino headers needed).
- All tests pass on Linux/GNUC.
- The build succeeds under PlatformIO's native-test environment.
- No existing functionality is broken.

### Notes
This is a **replacement** for the deleted test, not a new feature. It preserves the ability to verify timezone correctness without target hardware. Tests are deliberately minimal — only the table data and error paths that matter for production behavior (wrong offset = wrong displayed time).
