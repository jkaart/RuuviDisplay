# Task ID: TASK-002

## Title: Make timezone configurable via WiFiManager parameters (runtime reconfiguration)

### Summary
The current implementation hardcodes `"Europe/Helsinki"` in both `src/display.cpp` (two `utcToLocal(...)` calls) and `src/main.cpp` (NTPClient construction). This prevents deploying the same firmware image to devices in different timezones without recompiling. This task adds a configurable timezone parameter settable through the existing captive portal UI, with the new value taking effect immediately on the next NTP sync cycle.

### Context
Currently both files contain:
    utcToLocal("Europe/Helsinki", g_renderEpoch, &tmv);  // display.cpp (twice)
    static NTPClient ntpClient(ntpUdp, "pool.ntp.org", 0, 3600000);

The string literal `"Europe/Helsinki"` appears twice — changing it would require editing two places. More importantly, there is no mechanism to change it after flashing, so field technicians cannot adapt the firmware for a different region without building a custom image.

### Proposed Changes

1. **include/wifi_config.h** — Add timezone parameter definition:
    #define WIFI_MANAGER_PARAM_TIMEZONE "timezone"
    #define WIFI_MANAGER_PARAM_TIMEZONE_LABEL "Timezone (e.g., Europe/Helsinki)"
    #define WIFI_MANAGER_PARAM_TIMEZONE_DEFAULT "Europe/Helsinki"

2. **src/main.cpp** — Add the parameter and a save callback:
    - Add `WiFiManagerParameter timezoneParam("timezone", "Timezone", "Europe/Helsinki", 32);`
    - Add a `saveTimezoneCallback()` that writes to NVRAM and refreshes the effective config.
    - In `syncNtp()`, use the configured timezone string instead of the hardcoded literal.

3. **src/display.cpp** — Read the timezone from a global config (set by main.cpp) instead of hardcoding.

### Implementation Steps
1. Add parameter definition to `include/wifi_config.h`.
2. Modify `src/main.cpp`: add param, save callback, persist to NVRAM.
3. Modify `src/display.cpp`: use global timezone config.
4. Wire the save callback into WiFiManager setup.

### Acceptance Criteria
- A timezone can be changed in the captive portal UI.
- The new value takes effect immediately on the next NTP sync / display refresh cycle.
- Default remains `"Europe/Helsinki"` for backward compatibility.
- An empty/invalid value falls back to the default without crashing.
- Build succeeds and runtime behavior is unchanged when no parameter is set.
