#include <Arduino.h>

#include "wifi_config.h"

#include <WiFiManager.h>
#include <Preferences.h>

// -- Callbacks ---------------------------------------------------------------
void configModeCallback(WiFiManager *myWiFiManager);
void saveConfigCallback();   // WiFi credentials changed -> reboot + persist custom config
void saveParamsCallback();   // custom params (endpoint/api key) saved -> persist, no reboot

static bool g_shouldReboot = false;

// -- Captive portal custom fields (must outlive the portal session) ----------
WiFiManagerParameter httpEndpoint("http_endpoint", "Endpoint url", "", 64);
WiFiManagerParameter apiKeyParam("api_key", "API key", "", 64);

// -- Persistent storage for endpoint URL and API key -------------------------
AppConfig g_config;
Preferences prefs;

// Read the endpoint URL / API key from Preferences into g_config. Called once
// after a successful WiFi connection so the app always has current values.
void loadCustomConfig();

// Persist the endpoint URL and API key entered in the captive portal. Idempotent:
// only non-empty values are written, so calling it from either save callback is safe.
void persistCustomConfig()
{
  String endpoint = httpEndpoint.getValue();
  String apiKey   = apiKeyParam.getValue();

  prefs.begin(PREFERENCES_NAME, false);
  if (!endpoint.isEmpty()) {
    prefs.putString("endpoint_url", endpoint);
  } else {
    prefs.remove("endpoint_url");
  }
  if (!apiKey.isEmpty()) {
    prefs.putString("api_key", apiKey);
  } else {
    prefs.remove("api_key");
  }
  prefs.end();

  loadCustomConfig();
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("[wifi] Entered configuration mode");
  Serial.println("AP name: " + myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback()
{
  // New WiFi credentials were stored; a fresh boot is required to apply them.
  g_shouldReboot = true;
  persistCustomConfig();
}

void saveParamsCallback()
{
  persistCustomConfig();
}

void loadCustomConfig()
{
  if (!prefs.begin(PREFERENCES_NAME, false)) {
    return;
  }
  String endpoint = prefs.getString("endpoint_url", "");
  String apiKey   = prefs.getString("api_key", "");
  prefs.end();

  // Populate g_config (truncate to the fixed buffer sizes).
  endpoint.toCharArray(g_config.httpEndpoint, sizeof(g_config.httpEndpoint));
  apiKey.toCharArray(g_config.apiKey, sizeof(g_config.apiKey));
}

void setup()
{
  Serial.begin(115200);
  delay(1000); // give the serial port time to initialize before WiFiManager uses it
  Serial.println();
  Serial.println("[wifi] Starting up...");

  WiFiManager wifiManager;

#ifdef DEBUG
  wifiManager.setDebugOutput(true);
#else
  wifiManager.setDebugOutput(false);
#endif

  wifiManager.setAPCallback(&configModeCallback);
  wifiManager.setSaveConfigCallback(&saveConfigCallback);
  wifiManager.setSaveParamsCallback(&saveParamsCallback);

  wifiManager.addParameter(&httpEndpoint);
  wifiManager.addParameter(&apiKeyParam);

  // Blocking auto-connect: tries saved credentials, and if those fail (or none
  // are stored) opens the captive portal until a user saves new ones. Returns
  // control to this sketch once connected or after the configured timeout.
  if (!wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD))
  {
    Serial.println("[wifi] Could not establish a connection; rebooting to retry.");
    delay(3000);
    ESP.restart();
  }

  // New credentials were saved during the portal session and need a fresh boot.
  if (g_shouldReboot)
  {
    Serial.println("[wifi] Rebooting to apply new credentials...");
    delay(3000);
    ESP.restart();
  }

  // Load endpoint URL / API key persisted from the last portal session so the
  // app has them available immediately after connecting.
  loadCustomConfig();

  Serial.print("[wifi] Connected as STA, IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("[wifi] Configuration complete.");
}

void loop()
{
  // Idle for now. Future work: initialize the EPDiy display and run the sensor
  // update loop here (e.g. epd.init(); epd.updatePanel()).
}
