# Task ID: TASK-010

## Title: Complete the `<time.h>` → `<ctime>` refactor across all source files

### Summary
The previous commit (`969bf67`) partially refactored timezone include handling by changing
`include/display.h` from `#include <time.h>` to `#include <ctime>`. This change is safe and
correct for the Arduino/ESP32 platform, but it was applied inconsistently — three other files
still use `<time.h>`. This task completes the refactor by updating all remaining occurrences.

### Background

On ESP-IDF (the C/C++ framework underlying Arduino for ESP32), both headers provide identical
functionality:
- `time_t` type
- `gmtime_r()`, `localtime_r()` functions
- Other time-related declarations

The swap from `<time.h>` → `<ctime>` is a zero-risk, behavior-preserving change that improves
consistency and portability.

### Files to Update

Update each of the following files by replacing:

```c++
#include <time.h>   // time_t, struct tm
```

with:

```c++
#include <ctime>    // C++ header providing identical types/functions on ESP32/Arduino
```

The `//` comment may be updated to match the existing style (e.g., documenting what is used).

### Files

1. **src/RuuviMeasurement.cpp** — 1 line change
2. **src/display.cpp** — 1 line change  
3. **include/timezone.h** — 1 line change (plus comment update if desired)

`include/display.h` was already updated in the previous commit and does not need editing again.

### Verification Steps

After making all changes:

```bash
pio run -e lilygo-t5-47       # verify ESP32 build succeeds
g++ -std=c++17 -c src/*.cpp include/*.h  # native host sanity check
```

Confirm:
- No warnings or errors related to `<time.h>` / `<ctime>`
- The project still builds and runs correctly
- Runtime behavior is unchanged (the swap is semantically identical)

### Notes

- No test changes are needed; this is a pure header-name refactor.
- If PlatformIO build tools are unavailable in the environment, document the manual verification steps instead.
