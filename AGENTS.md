# RuuviDisplay Project - Agent Guidelines

## Agent Operating Rules

These rules take precedence over convenience when implementing features.

### 1. Investigate before implementing

Do not immediately start writing code when given a new feature request.

Before making changes:

1. Understand what the feature actually requires.
2. Inspect the existing project and current implementation.
3. Check whether the project already contains functionality that can be reused.
4. Check the existing PlatformIO dependencies.
5. Check whether Arduino or ESP32 already provides the required functionality.
6. Search for suitable external libraries when appropriate.
7. Compare the available options before choosing an implementation.

The goal is to reuse existing, well-tested functionality rather than
reimplementing it.

### 2. Library and platform-first approach

If a suitable, maintained library exists, prefer using it over writing
the functionality from scratch.

Do NOT implement a protocol, driver, parser, storage system, networking
component, or other substantial functionality manually if a suitable
library already exists.

When a feature could reasonably be provided by a library, actively
search for an existing library before considering a custom implementation.

Before adding a new dependency, verify:

- compatibility with ESP32
- compatibility with the Arduino framework
- compatibility with the project's PlatformIO/ESP32 version
- whether the library is actively maintained
- whether the required functionality is actually supported
- whether an existing project dependency can already provide it
- whether Arduino or ESP32 already provides the required functionality

Do not add a dependency merely because it is the first library found.

Do not implement functionality manually merely because a custom
implementation appears simple.

### 3. Do not silently choose an implementation

When there are multiple reasonable approaches, explain the alternatives
before making a substantial architectural choice.

Possible approaches include:

- existing project functionality
- Arduino/ESP32 built-in functionality
- existing project dependency
- mature external library
- custom implementation

Prefer, in this order:

1. Existing project functionality
2. Arduino/ESP32 built-in functionality
3. Existing project dependency
4. Mature external library
5. Custom implementation

A custom implementation should be the last resort.

If choosing a custom implementation over an existing library or built-in
functionality, explicitly explain why.

### 4. Separate investigation from implementation

For non-trivial features, use two phases.

Phase 1: Investigation

- inspect the project
- research relevant APIs and libraries
- identify compatibility issues
- compare viable approaches
- propose the implementation

Do not modify source files during this phase.

Phase 2: Implementation

Only after the approach is clear, modify the project.

After implementation:

- build the project
- run available tests
- fix compilation errors
- verify that the implementation follows the project architecture

For simple, low-risk changes where the correct implementation is
obvious and no architectural or dependency decision is required,
the agent may implement the change directly.

For anything involving a new dependency, library choice, hardware
interaction, architecture, protocol, persistence, networking, or
substantial code changes, the agent MUST stop after investigation
and present the proposed approach before modifying source files.

### 5. When the user asks to add functionality

When a user asks for a new feature, do not assume that the requested
implementation is the best implementation.

First determine:

- Is this functionality already present in the project?
- Is it provided by Arduino?
- Is it provided by ESP32/ESP-IDF?
- Is it provided by an existing dependency?
- Is there a suitable external library?
- Does the hardware require a specific implementation?

If there are multiple viable solutions, present the recommended
solution and the important alternatives before implementation.

Do not add a new library or create a custom implementation without
considering the alternatives.

## Overview

This is an ESP32-based PlatformIO project for the LilyGo T-Display (T5-47) e-Paper display device. The project uses Arduino framework with PlatformIO build system.

**Hardware:**
- ESP32-WROVER-KIT or compatible boards (ESP32/ESP32-S3)
- E-Ink e-Paper display (ED047TC1 - 4.7 inch)
- EPDiy library for e-paper control
- WiFiManager for web-based configuration

**Location:** `~/Documents/PlatformIO/Projects/RuuviDisplay`

---

## Project Structure

```
RuuviDisplay/
├── old/                          # Reference: working demo (DO NOT MODIFY)
│   ├── src/main.cpp             # Working reference implementation
│   ├── src/pins.h               # Pin definitions for T5-47 boards
│   └── platformio.ini           # PlatformIO configuration
├── include/                      # Project headers
├── lib/                          # Project libraries
├── src/                          # Source files (create as needed)
└── test/                         # Test files
```

---

## Key Files & Configuration

### PlatformIO Configuration (`platformio.ini`)

**Common Settings:**
- Platform: `espressif32`
- Framework: `arduino`
- Board: `esp-wrover-kit` (or compatible)
- Upload speed: `921600`
- Monitor speed: `115200`

**Build Flags:**
```ini
-DBOARD_HAS_PSRAM              # Enable PSRAM for display buffer
-DCONFIG_EPD_DISPLAY_TYPE_ED047TC1  # ED047TC1 e-paper model
-DCONFIG_EPD_BOARD_REVISION_LILYGO_T5_47  # LilyGo T-Display board
-D CORE_DEBUG_LEVEL=3          # Debug level
```

**Dependencies:**
- `tzapu/WiFiManager@^2.0.17` - Web configuration server
- `vroland/epdiy@^2.1.3` - e-Paper display library

---

## Hardware Pin Definitions

### T5-47 Board (ESP32)
| Function | Pin |
|----------|-----|
| BUTTON_1 | 34 |
| BUTTON_2 | 35 |
| BUTTON_3 | 39 |
| BATT_PIN | 36 |
| SD_MISO | 12 |
| SD_MOSI | 13 |
| SD_SCLK | 14 |
| SD_CS | 15 |

### T5-47 Plus Board (ESP32-S3)
| Function | Pin |
|----------|-----|
| BUTTON_1 | 21 |
| BATT_PIN | 14 |
| SD_MISO | 16 |
| SD_MOSI | 15 |
| SD_SCLK | 11 |
| SD_CS | 42 |

---

## Development Workflow

### 1. Initial Setup

```bash
cd ~/Documents/PlatformIO/Projects/RuuviDisplay
pio run -t build          # Build project
pio run -t upload         # Upload to board
pio run -t monitor        # Monitor serial output
```

### 2. Reference Implementation Study

The `old/src/main.cpp` contains a working demo using:
- **IotWebConf**: Captive portal web server for configuration
- **WiFi AP mode**: Initial setup with password "smrtTHNG8266"
- **HTTP server**: Port 80, serves configuration page at `/config`

**Key features from reference:**
- WiFi connection callback handling
- Form validation for HTTP server parameter
- Configuration persistence via IotWebConf
- Captive portal support

### 3. Adding New Features

Every feature should follow this workflow:

#### Step 1 — Understand

Determine:
- What is the actual requirement?
- What existing code is related to it?
- What constraints does the hardware impose?

#### Step 2 — Investigate

Before writing code:
- search the repository
- inspect existing dependencies
- inspect Arduino/ESP32 APIs
- search for suitable libraries if necessary

#### Step 3 — Plan

Determine the smallest appropriate implementation.

For non-trivial features, STOP after the investigation phase and
present the proposed approach before modifying source files.

Do not modify source files during investigation unless explicitly
asked to do so.

#### Step 4 — Implement

Only after the approach is established:
- modify/create files in `src/`
- reuse existing code where appropriate
- use libraries instead of custom implementations where appropriate
- keep functionality modular

#### Step 5 — Verify

Run:

    pio run

and, when applicable:

    pio test

Fix any errors introduced by the implementation.

#### Step 6 — Summarize

Report:
- what changed
- which libraries were added or used
- why they were chosen
- what was tested
- any remaining limitations

### 4. Testing

```bash
pio run -t test           # Run tests if available
pio run -t monitor        # Monitor for debugging
```

---

## Common Tasks

### Build & Upload
```bash
# Full build and upload
pio run

# Specific target
pio run -t build
pio run -t upload
pio run -t clean          # Clean build artifacts
```

### Debugging
```bash
# Monitor with auto-reset
pio run -t monitor --follow

# Upload with verbose output
pio run -t upload -v
```

---

## Important Notes

⚠️ **DO NOT MODIFY** files in the `old/` folder - it contains reference code only.

✅ **CREATE NEW FILES** in:
- `src/` - Main source code
- `include/` - Header files
- `lib/` - Project-specific libraries
- `test/` - Test cases

📝 **When referencing old code:**
- Copy patterns from `old/src/main.cpp` as starting point
- Adapt pin definitions from `old/src/pins.h` for your board variant
- Study IotWebConf usage in the reference implementation

---

## Board Selection

To compile for different boards, modify `platformio.ini`:

```ini
[env:esp-wrover-kit]
board = esp-wrover-kit    # LilyGo T-Display 4.7"

[env:t-display-v2]
board = t-display-v2      # Alternative board if needed
```

---

## External Resources

- **PlatformIO Docs**: https://docs.platformio.org/
- **Arduino ESP32 Core**: https://github.com/espressif/arduino-esp32
- **WifiManager**: https://github.com/tzapu/WiFiManager
- **EPDiy Library**: https://github.com/vroland/epdiy

---

## Quick Start Checklist

- [ ] Verify board is connected via USB
- [ ] Run `pio run` to build and upload
- [ ] Open monitor with `pio run -t monitor`
- [ ] Check serial output for "Ready." message
- [ ] Access web interface at device IP (if WiFi configured)
