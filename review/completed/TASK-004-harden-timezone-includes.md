# Task ID: TASK-004

## Title: Harden timezone include handling for ESP-IDF/Arduino platform compatibility

### Summary
`include/display.h` now includes `<time.h>` to use `time_t`. On the Arduino/ESP32 platform (ESP-IDF toolchain), C standard headers like `<time.h>` are not always directly available in the Arduino framework's environment. This task ensures timezone functionality compiles reliably across all target environments without requiring special PlatformIO flags or conditional-compilation hacks.

### Context
The current change adds:
    #include <time.h>   // time_t: NTP epoch captured at render time, shown in "Last updated"
to `include/display.h`. While this compiles on GCC-hosted environments (for testing), it may fail to compile when building under PlatformIO with the Arduino framework targeting ESP32.

### Proposed Changes
**Option A — Prefer `<ctime>` over `<time.h>`:**
`<ctime>` is the C++ header that wraps `time.h` functionality. The Arduino ESP32 core exposes `<ctime>` (which provides `time_t`, `gmtime()`, `localtime()`, etc.) while potentially lacking direct access to `<time.h>`. Swapping the include resolves the issue without changing behavior.

**Option B — Conditional include (safer):**
If both headers might be needed depending on platform, guard so `<time.h>` is only included on the native toolchain:
    #include <ctime>          // primary: always available in Arduino C++ environment
#ifdef __linux__
#  if !defined(__arm__)
#    include <time.h>     // fallback for native toolchain (GCC/Clang on Linux)
#  endif
#endif

**Recommendation:** Use **Option A** (`<ctime>`). The existing code uses `time_t` and `gmtime_r()/localtime_r()`, which are part of the C++17 `<ctime>` header. This is a zero-risk swap that improves portability.

### Implementation Steps
1. Edit `include/display.h`: replace `#include <time.h>` with `#include <ctime>`.
2. Optionally verify `src/timezone.cpp` (it includes `<Timezone.h>` from the library) doesn't conflict.
3. Verify the build succeeds for both `pio run -e lilygo-t5-47` and a native-host build (`g++ -std=c++17 ...`).

### Acceptance Criteria
- Build succeeds with `pio run -e lilygo-t5-47`.
- No compilation warnings or errors related to `<time.h>`/`<ctime>`.
- Runtime behavior is unchanged (same output for the "Last updated" row).

## Resolution

**Approach:** Implemented **Option A** (recommended in the task). Replaced
`#include <time.h>` with `#include <ctime>` in `include/display.h`. This is a
zero-risk, behavior-preserving swap: the Arduino-ESP32 core (ESP-IDF) exposes
`time_t`, `gmtime_r()`, and `localtime_r()` through the C++ `<ctime>` header,
and these same functions/typedefs are what the rest of the code uses.

**Change made:**
- `include/display.h`: line 4 — `#include <time.h>` → `#include <ctime>`.

**Files checked (no change needed):**
- `src/timezone.cpp` — includes `<Timezone.h>` (jchristensen/Timezone lib) and
  uses `gmtime_r()`; unaffected by the header swap and compiles cleanly.
- `include/timezone.h`, `src/display.cpp`, `src/RuviMeasurement.cpp` — these use
  pre-existing `#include <time.h>` and were already compiling on the ESP32
  Arduino core; they are outside the scope of this task and were left untouched.

**Tests / verification run:**
- `pio run -e lilygo-t5-47` (full ESP32/Arduino build) — **SUCCESS**, after a
  clean build (`-t clean`) so the `<ctime>` change in the header was recompiled
  from scratch. No errors or warnings related to `<time.h>`/`<ctime>`.
- Native host sanity check: `g++ -std=c++17` confirms `<ctime>` provides
  `time_t` + `gmtime_r` (the reverse direction).
- The `test` (native) environment could not be run because its referenced test
  sources are absent from the repo (pre-existing; not caused by this change and
  intentionally left alone).

**Acceptance criteria:**
- [x] Build succeeds with `pio run -e lilygo-t5-47`.
- [x] No compilation warnings or errors related to `<time.h>`/`<ctime>`.
- [x] Runtime behavior is unchanged (pure include swap; `<ctime>` exposes the
      same `time_t`/`gmtime_r`/`localtime_r` semantics on ESP32).

**Status:** completed.
