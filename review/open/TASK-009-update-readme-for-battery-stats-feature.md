# Task ID: TASK-009

status: open

## Title: Update README.md for the local 18650 cell voltage feature and new dependency

### Summary
The `feat/lilygo-battery-stats` branch added a user-visible feature — the local display
board's 18650 cell voltage shown at the bottom-right of the e-paper panel — and a new
external dependency (`danilopinotti/Battery_18650_Stats`), but `README.md` was not updated.
The README's Features list, Display Layout section and Software / dependency sections are
now stale.

### Context

Verified against the branch changes:

- `platformio.ini` adds `danilopinotti/Battery_18650_Stats@^1.0.0` to
  `common.lib_deps_external`.
- `src/main.cpp` reads the board cell voltage (ADC pin 36) once per boot and logs it as
  `[battery] Voltage: ...`.
- `src/display.cpp` renders the voltage, right-aligned on the same row as the "Last
  updated" row at the bottom of the panel; it is also shown on the error path.
- `README.md` currently:
  - lists dependency names under "Software" (WiFiManager, epdiy, ArduinoJson, Timezone,
    NTPClient is missing too) without `Battery_18650_Stats`;
  - the Display Layout mock diagram (lines ~180-193) shows a blank bottom row;
  - the Features list has no mention of the local battery voltage display.

### Proposed Changes

**`README.md`**:

- Add a feature bullet for the local 18650 cell voltage display (e.g., "Local battery
  (18650) cell voltage shown at the bottom of the display").
- Update the Display Layout section and simplified ASCII diagram to include the bottom row
  with "Last updated" (left) and cell voltage (right).
- Add `Battery_18650_Stats` to the "Software" dependency list and the inline
  `platformio.ini` snippet (the snippet is already missing NTPClient; add it only if it
  stays accurate, otherwise at minimum add the new library).

Do not create new documentation files; only edit the existing `README.md`.

### Acceptance Criteria
- `README.md` describes the local battery voltage feature.
- `Battery_18650_Stats` appears in the dependency lists/snippet in `README.md`.
- No source code is modified by this task.