# Task ID: TASK-007

status: open

## Title: Correct the misleading battery conversion factor comment and document its provenance

### Summary
`src/main.cpp` defines `BATTERY_CONVERSION_FACTOR` as `1.795` and labels it "T18 factory
value". That label is factually wrong and the branch history contains contradictory
calibration claims. The comment must be corrected so future maintainers do not treat the
value as an off-the-shelf T18 constant when it is a board- and wiring-specific factor that
still needs a multimeter verification.

### Context

The following was verified against the actual code and repository history:

1. The `Battery_18650_Stats` library (danilopinotti/Battery_18650_Stats@1.0.0) defines the
   T18 (LILYGO-T-Energy) factors as `DEFAULT_PIN 35` and `DEFAULT_CONVERSION_FACTOR 1.702`
   (see `.pio/libdeps/lilygo-t5-47/Battery_18650_Stats/src/Battery18650Stats.h`). The T18
   factory value is therefore **1.702 on pin 35**, not 1.795.
2. The previous working implementation for this exact board (`old/src/main.cpp:59`) used
   `Battery18650Stats battery(BATT_PIN, 1.79)` with `BATT_PIN 36` (`old/src/pins.h:17`).
   The value that matches this board is therefore the locally calibrated **~1.79**, which
   is what the branch's final `1.795` approximates.
3. The branch commits contradict themselves without a documented basis:
   - `3e28784`: introduced `1.79` (comment: "T18 factory value")
   - `3341bde`: "correct battery conversion factor" → changed to `1.598`
   - `de585f4`: "update battery conversion factor to improve voltage accuracy" → `1.795`
   No commit provides multimeter measurements; `1.598` would under-report a full 4.2 V cell
   by ~10%. The final value was never verified against a physical measurement in the branch.
4. The same wrong label is duplicated in the second comment block at the
   `static Battery18650Stats g_battery(36, BATTERY_CONVERSION_FACTOR);` declaration
   (`src/main.cpp:73-75`), which says the "T18 factory scaling factor is used until adjusted".

### Proposed Changes

**`src/main.cpp`** — replace both comment blocks so they state actual provenance:

- State that `BATTERY_CONVERSION_FACTOR` derives from the locally calibrated value used by
  the previous T5-47 implementation (~1.79 / 1.795), **not** from the T18 factory default
  (1.702) or from any verified measurement in this branch.
- State explicitly that the factor is unverified and must be calibrated against a
  multimeter (the fix commits in this branch toggled the value twice without calibration
  evidence).
- Optionally: replace the `#define` with a documented constant (e.g.,
  `static constexpr double BATTERY_CONVERSION_FACTOR = 1.795;` with a comment referencing
  the calibration requirement) so the value's purpose and required verification steps are
  visible at the point of use.

Do not change the value `1.795` itself — the task is documentation accuracy. Re-validate
the factor against a multimeter readout of the 18650 cell as a separate manual step if a
device is available.

### Acceptance Criteria
- Both comment blocks in `src/main.cpp` no longer claim "T18 factory value".
- The comment identifies the actual source of the factor (previous T5-47 calibration ~1.79)
  and states that a multimeter calibration is still required.
- The value of `BATTERY_CONVERSION_FACTOR` is unchanged.
- Firmware build succeeds: `pio run -e lilygo-t5-47`.