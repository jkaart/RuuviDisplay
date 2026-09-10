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
