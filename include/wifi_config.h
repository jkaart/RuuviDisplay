#pragma once

// WiFi configuration for the provisioning access point created by WiFiManager.
// These values are shown to a user when the device boots without saved
// credentials and opens its captive portal (default gateway 192.168.4.1).

const char* const WIFI_AP_NAME = "RuuviDisplay";

// WPA2 password for the provisioning AP. Must be >= 8 ASCII characters.
const char* const WIFI_AP_PASSWORD = "ruuvi12345";

// Marker used to distinguish configuration versions when persisting settings.
// Bumped: captive portal now also accepts an HTTP endpoint URL and API key.
#define CONFIG_VERSION "wifi2"

// Namespace under which the endpoint URL / API key are stored in Preferences.
#define PREFERENCES_NAME "ruuvidisplay"

// Runtime application configuration, populated from the WiFiManager captive
// portal (Endpoint url + API key) and persisted across reboots via Preferences.
// The app/sensor layer reads these values to know where to send data and which
// API key to use. Empty fields mean "not configured".
struct AppConfig {
    char httpEndpoint[65];  // e.g. https://api.example.com/ingest
    char apiKey[65];         // API key for the endpoint
};

extern AppConfig g_config;
