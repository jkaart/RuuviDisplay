# RuuviDisplay Project - Agent Guidelines

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

When implementing new functionality:

1. **Create source files in `src/` directory** (not in `old/`)
2. **Use pins.h from old/src/pins.h as reference** for pin definitions
3. **Include necessary libraries** from PlatformIO dependencies or external libs
4. **Follow Arduino best practices**:
   - Initialize in `setup()`
   - Implement logic in `loop()`
   - Use non-blocking code (millis() instead of delay())

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
