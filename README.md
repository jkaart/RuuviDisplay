# RuuviDisplay

RuuviDisplay is an ESP32-based e-paper display for showing data from [RuuviTag](https://ruuvi.com/) sensors.

The project is designed for the **LilyGO T5 4.7" e-paper display** and uses the EPDiy library to drive the 960 × 758 ED047TC1 display.

The device connects to Wi-Fi, retrieves the latest RuuviTag measurements from a configurable HTTPS backend API, displays the data on the e-paper screen, and then enters deep sleep to minimize power consumption.

## Features

- Designed for the **LilyGO T5 4.7" e-paper display**
- 960 × 758 pixel landscape e-paper display
- Displays up to **three RuuviTag sensors**
- Temperature
- Relative humidity
- Atmospheric pressure
- Battery voltage
- RuuviTag name and MAC address
- Measurement timestamp
- Automatic **Europe/Helsinki** timezone and daylight-saving-time handling
- Wi-Fi configuration using a captive portal
- Persistent Wi-Fi and backend configuration
- Configurable backend URL
- Configurable API key
- HTTPS-only backend communication
- Backend health check before updating the display
- Automatic retry when the backend is unavailable
- Deep sleep between display updates
- Approximately **30-minute update cycle** during normal operation
- Short retry cycle when the backend cannot be reached

## Hardware

The project is designed for:

- **LilyGO T5 4.7"**
- ESP32-WROVER based hardware
- 16 MB flash
- 8 MB PSRAM
- ED047TC1 4.7" e-paper panel

The project uses the `esp-wrover-kit` PlatformIO board definition while explicitly configuring the flash size to 16 MB.

The display is initialized in landscape orientation.

## How It Works

The device follows this basic cycle:

```text
                    ┌──────────────────┐
                    │      Boot        │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │ Connect to Wi-Fi │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │  Load settings   │
                    │  from storage    │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │ HTTPS /health    │
                    │     check        │
                    └────────┬─────────┘
                             │
                    ┌────────┴────────┐
                    │                 │
                  Failed              OK
                    │                 │
                    ▼                 ▼
             ┌─────────────┐   ┌──────────────┐
             │ Deep sleep  │   │ GET /api     │
             │  ~2 minutes │   │ Ruuvi data   │
             └──────┬──────┘   └──────┬───────┘
                    │                 │
                    │                 ▼
                    │          ┌──────────────┐
                    │          │ Parse JSON   │
                    │          └──────┬───────┘
                    │                 │
                    │                 ▼
                    │          ┌──────────────┐
                    │          │ Update       │
                    │          │ e-paper      │
                    │          └──────┬───────┘
                    │                 │
                    │                 ▼
                    │          ┌──────────────┐
                    │          │ Deep sleep   │
                    │          │ ~30 minutes  │
                    │          └──────┬───────┘
                    │                 │
                    └─────────────────┘
                              reboot
```

The e-paper panel retains its image while the ESP32 is sleeping, so the display does not need to remain powered continuously.

## Backend API

RuuviDisplay does not communicate directly with RuuviTags.

Instead, it expects a backend service that provides the latest measurements through an HTTP API.

The backend must provide:

### Health endpoint

```http
GET /health
```

A successful HTTP `200 OK` response indicates that the backend is available.

The response may contain a JSON object such as:

```json
{
  "status": "ok"
}
```

The current firmware primarily uses the HTTP status code to determine whether the endpoint is reachable.

### Ruuvi data endpoint

```http
GET /api
```

The API key is sent using the `x-api-key` HTTP header:

```http
x-api-key: YOUR_API_KEY
```

The endpoint is expected to return a JSON array containing RuuviTag measurements.

Example:

```json
[
  {
    "name": "Living Room",
    "mac": "AA:BB:CC:DD:EE:FF",
    "temperature": 21.42,
    "humidity": 45.73,
    "pressure": 101325,
    "batteryVoltage": 2.987,
    "timestamp": 1750000000
  }
]
```

Pressure is expected in **Pa** and is converted to **hPa** for display.

The firmware keeps the latest measurement for each unique MAC address.

## Display Layout

The 960 × 758 display is divided into three sensor panels.

Each panel contains:

- RuuviTag name
- Temperature
- Humidity
- Pressure
- Battery voltage
- Measurement timestamp

A simplified representation:

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│                                                                              │
│        Living Room          Bedroom             Outdoor                      │
│                                                                              │
│  🌡 21.42 °C           🌡 20.18 °C         🌡 12.73 °C                       │
│  💧 45.73 %RH          💧 51.21 %RH        💧 78.42 %RH                      │
│  ⏱ 1013.2 hPa         ⏱ 1012.8 hPa       ⏱ 1008.4 hPa                     │
│  🔋 2.987 V            🔋 2.943 V          🔋 2.801 V                        │
│                                                                              │
│  01/09/26 18:30:12     01/09/26 18:29:58   01/09/26 18:29:41                 │
│                                                                              │
└──────────────────────────────────────────────────────────────────────────────┘
```

The actual display uses custom bitmap icons and Open Sans fonts.

## Wi-Fi Configuration

Wi-Fi is configured using [WiFiManager](https://github.com/tzapu/WiFiManager).

On the first boot, or when saved Wi-Fi credentials are unavailable, the device creates a configuration access point:

```text
SSID: RuuviDisplay
Password: ruuvi12345
```

The captive portal is available through:

```text
192.168.4.1
```

The configuration portal is used to configure:

- Wi-Fi credentials
- Backend URL
- API key

The backend URL and API key are stored persistently using ESP32 `Preferences`, so they survive reboots and deep-sleep cycles.

> **Security note:** Change the default provisioning AP password if the device will be used in an environment where other people can access it.

## HTTPS

Backend communication is forced to HTTPS.

For example:

```text
example.com
```

becomes:

```text
https://example.com
```

and:

```text
http://example.com
```

is converted to:

```text
https://example.com
```

The firmware therefore does not intentionally send the backend API key over plain HTTP.

## Power Management

RuuviDisplay is designed to spend most of its time sleeping.

During a normal cycle:

1. ESP32 boots.
2. Wi-Fi is connected.
3. Backend configuration is loaded.
4. The backend health endpoint is checked.
5. Latest RuuviTag data is downloaded.
6. The e-paper display is updated.
7. Wi-Fi sleep is enabled.
8. The ESP32 enters deep sleep for approximately 30 minutes.
9. The device wakes by rebooting and starts the cycle again.

If the backend is unavailable, the display is not unnecessarily updated. Instead, the ESP32 enters a shorter deep-sleep period of approximately two minutes and retries.

## Software

The project uses:

- **PlatformIO**
- **Arduino framework**
- **ESP32**
- **EPDiy**
- **WiFiManager**
- **ArduinoJson**
- **Timezone**

The main dependencies are defined in `platformio.ini`:

```ini
tzapu/WiFiManager
vroland/epdiy
bblanchon/ArduinoJson
jchristensen/Timezone
```

The current project configuration uses the `pioarduino` ESP32 platform and configures the board for a 16 MB flash device.

## Building

Install [PlatformIO](https://platformio.org/) and clone the repository:

```bash
git clone https://github.com/jkaart/RuuviDisplay.git
cd RuuviDisplay
```

Build the project:

```bash
pio run
```

Upload it to the ESP32:

```bash
pio run -t upload
```

Open the serial monitor:

```bash
pio run -t monitor
```

The default serial speed is:

```text
115200 baud
```

## Flash Configuration

The LilyGO T5 4.7" hardware used by this project has 16 MB of flash.

The PlatformIO configuration therefore explicitly sets:

```ini
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
board_build.partitions = default_16MB.csv
```

PSRAM is also enabled:

```ini
-DBOARD_HAS_PSRAM
```

The framebuffer required by the e-paper display is stored using the available PSRAM.

## Project Structure

```text
RuuviDisplay/
├── include/
│   ├── display.h
│   ├── timezone.h
│   └── wifi_config.h
│
├── lib/
│
├── src/
│   ├── main.cpp
│   ├── display.cpp
│   ├── RuuviMeasurement.cpp
│   ├── RuuviMeasurement.h
│   ├── epd_driver.h
│   ├── timezone.cpp
│   └── timezone_table.cpp
│
├── test/
│
├── old/
│   └── # Reference implementation
│
├── AGENTS.md
└── platformio.ini
```

The `old/` directory contains reference material from the earlier implementation and is not part of the current application architecture.

## Architecture

The firmware is divided into a few main responsibilities.

### `main.cpp`

Handles:

- ESP32 startup
- Wi-Fi provisioning
- persistent configuration
- backend connectivity
- HTTPS requests
- deep sleep
- application lifecycle

### `RuuviMeasurement`

Handles:

- JSON parsing
- extracting RuuviTag measurements
- identifying sensors by MAC address
- keeping the latest measurement for each sensor

### `display.cpp`

Handles:

- EPDiy initialization
- framebuffer management
- e-paper rendering
- sensor icons
- fonts
- display layout
- local timestamps

### Timezone Support

Timestamps received from the backend are treated as UTC and converted to:

```text
Europe/Helsinki
```

The conversion is daylight-saving-time aware.

## Current Limitations

- A maximum of **three RuuviTags** are displayed.
- The display layout is currently designed specifically for the 960 × 758 LilyGO T5 4.7" panel.
- The firmware expects the backend API to provide the required RuuviTag fields.
- There is currently no direct Bluetooth communication with RuuviTags.
- The device depends on a network connection and an available backend API.
- The Wi-Fi provisioning password is currently defined in the firmware.

## Development

The project follows a library-first approach and uses existing Arduino/ESP32 functionality where possible.

Before adding significant functionality, check:

1. Existing project code
2. Arduino functionality
3. ESP32 functionality
4. Existing dependencies
5. Suitable external libraries

Build the project after changes:

```bash
pio run
```

Run tests when applicable:

```bash
pio test
```

Monitor the device during development:

```bash
pio run -t monitor
```

## Credits

This project makes use of the following open-source projects:

- [EPDiy](https://github.com/vroland/epdiy) — e-paper display driver
- [WiFiManager](https://github.com/tzapu/WiFiManager) — Wi-Fi provisioning
- [ArduinoJson](https://arduinojson.org/) — JSON parsing
- [Timezone](https://github.com/JChristensen/Timezone) — timezone and DST handling
- [PlatformIO](https://platformio.org/) — build system

## Status

RuuviDisplay is a personal ESP32/e-paper project and is actively developed.

The current implementation is focused on providing a simple, low-power display for RuuviTag sensor data using a remote backend API.
