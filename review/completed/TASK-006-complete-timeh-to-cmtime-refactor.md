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

## Resolution

**Approach:** Completed the existing `<time.h>` → `<ctime>` refactor by applying
the same zero-risk, behavior-preserving header swap to the three files that still
used `<time.h>`. On the Arduino/ESP32 (ESP-IDF) platform, `<ctime>` provides the
identical `time_t`, `struct tm`, `gmtime_r()`, and `localtime_r()` used throughout
the code, so this is semantically identical to `<time.h>`.

**Change made:**
- `src/RuuviMeasurement.cpp` (line 7): `#include <time.h>` → `#include <ctime>`.
- `src/display.cpp` (line 19): `#include <time.h>` → `#include <ctime>` (left the
  adjacent `#include <stdio.h>` untouched, per the task scope).
- `include/timezone.h` (line 3): `#include <time.h>   // time_t, struct tm` →
  `#include <ctime>    // time_t, struct tm` (comment preserved).

**Files checked (no change needed):**
- `include/display.h` — already updated in the previous commit (`969bf67`); verified
  it uses `<ctime>` and required no further edits.

**Tests / verification run:**
- `pio run -e lilygo-t5-47` (full ESP32/Arduino build) — **SUCCESS**. All three
  changed files (`RuuviMeasurement.cpp`, `display.cpp`, `timezone.h`) compiled
  cleanly from scratch with no warnings or errors related to `<time.h>`/`<ctime>`.
- Native host syntax check: `g++ -std=c++17 -Iinclude -fsyntax-only include/timezone.h`
  compiles cleanly (the only warning is the benign `#pragma once in main file`
  from directly compiling a header), confirming `<ctime>` exposes `time_t` and
  `struct tm`.
- Repo-wide grep confirms no `#include <time.h>` remains in `src/` or `include/`.
- The native `test` environment is not runnable because its referenced test sources
  (`test/timezone_table.cpp`, `test/test_timezone_table.cpp`) are absent from the
  repo — pre-existing and unrelated to this change; the task specifies no test
  changes are required.

**Acceptance criteria:**
- [x] All three remaining files (`src/RuuviMeasurement.cpp`, `src/display.cpp`,
      `include/timezone.h`) updated from `<time.h>` to `<ctime>`.
- [x] No remaining `<time.h>` includes in `src/` or `include/`.
- [x] `pio run -e lilygo-t5-47` builds successfully with no `<time.h>`/`<ctime>`
      warnings or errors.
- [x] Runtime behavior unchanged (pure header-name swap; identical semantics on ESP32).

**Status:** completed.
