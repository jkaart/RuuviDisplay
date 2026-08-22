#pragma once

// WiFi configuration for the provisioning access point created by WiFiManager.
// These values are shown to a user when the device boots without saved
// credentials and opens its captive portal (default gateway 192.168.4.1).

const char* const WIFI_AP_NAME = "RuuviDisplay";

// WPA2 password for the provisioning AP. Must be >= 8 ASCII characters.
const char* const WIFI_AP_PASSWORD = "ruuvi12345";

// Marker used to distinguish configuration versions when persisting settings.
#define CONFIG_VERSION "wifi1"
