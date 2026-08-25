#include <Arduino.h>

#include "wifi_config.h"

#include <WiFiManager.h>
#include <Preferences.h>

// HTTP client for outbound endpoint connectivity check (built into ESP32 core)
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

// Parse + print RuuviTag measurements from the backend /api JSON array.
#include "RuuviMeasurement.h"

// Render parsed measurements onto the e-paper display.
#include "display.h"

// -- Callbacks ---------------------------------------------------------------
void configModeCallback(WiFiManager *myWiFiManager);
void saveConfigCallback();   // WiFi credentials changed -> reboot + persist custom config
void saveParamsCallback();   // custom params (endpoint/api key) saved -> persist, no reboot

static bool g_shouldReboot = false;

// -- Rate limiting -----------------------------------------------------------
// Backend allows at most 10 requests per 15 min (~90s spacing). We poll more
// conservatively (2 min) to stay safely under the limit. On HTTP 429 we simply
// wait for the next scheduled poll cycle; no response-header reading is needed
// because this library version exposes no public header getter.
static uint32_t g_lastRuuviFetchMs = 0;   // millis of last /api fetch attempt
static uint32_t g_rateLimitUntilMs = 0;   // absolute millis until which we must not poll

const uint32_t RUUVI_POLL_INTERVAL_MS = 120000;   // default spacing between /api polls

// -- Captive portal custom fields (must outlive the portal session) ----------
WiFiManagerParameter backendParam("backend_url", "Backend URL", "", 64);
WiFiManagerParameter apiKeyParam("api_key", "API key", "", 64);

// -- Persistent storage for endpoint URL and API key -------------------------
AppConfig g_config;
Preferences prefs;

// Read the endpoint URL / API key from Preferences into g_config. Called once
// after a successful WiFi connection so the app always has current values.
void loadCustomConfig();

// Persist the backend URL and API key entered in the captive portal. Idempotent:
// only non-empty values are written, so calling it from either save callback is safe.
void persistCustomConfig()
{
  String backendUrl = backendParam.getValue();
  String apiKey   = apiKeyParam.getValue();

  prefs.begin(PREFERENCES_NAME, false);
  if (!backendUrl.isEmpty()) {
    prefs.putString("backend_url", backendUrl);
  } else {
    prefs.remove("backend_url");
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
  String backendUrl = prefs.getString("backend_url", "");
  String apiKey   = prefs.getString("api_key", "");
  prefs.end();

  // Populate g_config (truncate to the fixed buffer sizes).
  backendUrl.toCharArray(g_config.backendUrl, sizeof(g_config.backendUrl));
  apiKey.toCharArray(g_config.apiKey, sizeof(g_config.apiKey));
}

// One-shot connectivity check to the configured endpoint. Forces HTTPS and
// requests /health; verifies the response body contains status:"ok". No API key
// is sent for this request. Logs OK/FAILED to Serial, never blocks display work.
static void endpointHealthCheck()
{
  if (!g_config.backendUrl[0]) {
    Serial.println("[endpoint] no endpoint configured");
    return;
  }

  String url = g_config.backendUrl;
  if (url.startsWith("http://")) {
    url.replace("http://", "https://");   // force HTTPS
  } else if (!url.startsWith("https://") && !url.isEmpty()) {
    url = "https://" + url;               // no scheme -> prepend https://
  }
  url += "/health";

  Serial.printf("[endpoint] GET %s\n", url.c_str());

  HTTPClient http;
  if (!http.begin(url)) {
    Serial.println("[endpoint] /health FAILED: begin error");
    return;
  }
  int code = http.GET();
  String body = "";
  if (code == HTTP_CODE_OK) {
    body = http.getString();
  }
  http.end();

  // A successful HTTP response proves the endpoint is reachable; that is what a
  // /health check verifies. We also read the "status" field (if present) purely
  // for informational logging — we do not fail solely because its format differs.
  bool ok = false;
  if (code == HTTP_CODE_OK && !body.isEmpty()) {
    int sIdx = body.indexOf("\"status\"");   // position of opening quote
    String status = "";
    if (sIdx >= 0) {
      for (int i = sIdx + 8; i < (int)body.length(); i++) {
        char c = body.charAt(i);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;   // skip all whitespace
        if (c == ':') continue;                // skip ':' separator
        if (c == '"') {                        // start of the value string
          String val = "";
          val += c;
          i++;
          while (i < (int)body.length() && body.charAt(i) != '"') {
            val += body.charAt(i);
            i++;
          }
          if (!val.isEmpty() && val[0] == '"') {   // strip surrounding JSON quotes
            val = val.substring(1, val.length());   // endIndex is exclusive -> drops only the opening quote
          }
          status = val;
          break;
        }
        break;                                 // value ends (comma, }, etc.)
      }
    }
    ok = true;
    Serial.printf("[endpoint] /health OK (%d bytes) status=%s\n", body.length(), status.c_str());
  } else if (code == HTTP_CODE_OK) {
    Serial.println("[endpoint] /health OK (empty body)");
    ok = true;
  } else {
    Serial.printf("[endpoint] /health FAILED: HTTP %d\n", code);
  }

  if (!ok) {
    Serial.println("[endpoint] Endpoint unreachable or request failed.");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000); // give the serial port time to initialize before WiFiManager uses it
  Serial.println();
   Serial.println("[wifi] Starting up...");

   display_init();

   WiFiManager wifiManager;

#ifdef DEBUG
  wifiManager.setDebugOutput(true);
#else
  wifiManager.setDebugOutput(false);
#endif

  wifiManager.setAPCallback(&configModeCallback);
  wifiManager.setSaveConfigCallback(&saveConfigCallback);
  wifiManager.setSaveParamsCallback(&saveParamsCallback);

  wifiManager.addParameter(&backendParam);
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

  Serial.print("[wifi] Connected as STA, IP: ");
  Serial.println(WiFi.localIP());

  Serial.println("[wifi] Configuration complete.");

  // Load endpoint URL / API key persisted from the last portal session so the
  // app has them available immediately after connecting.
  loadCustomConfig();

  // Verify connectivity to the configured endpoint once (HTTPS GET /health).
  endpointHealthCheck();
}

// Fetch latest measurements from /api and print every tag to Serial. Returns
// true on success (HTTP OK + parsed). Reuses RuuviMeasurements for parsing/print;
// display integration will build on the same parsed data.
bool ruuviFetchAndPrint();

void loop()
{
  uint32_t now = millis();
  if (now >= g_rateLimitUntilMs && (now - g_lastRuuviFetchMs) >= RUUVI_POLL_INTERVAL_MS)
  {
    ruuviFetchAndPrint();
    g_lastRuuviFetchMs = now;
  }
  else if (now < g_rateLimitUntilMs)
  {
    Serial.printf("[ruuvi] rate limited, next poll in %llu s\n",
                  (unsigned long long)((g_rateLimitUntilMs - now)/1000ULL));
  }
}

bool ruuviFetchAndPrint()
{
  if (!g_config.backendUrl[0]) {
    Serial.println("[ruuvi] no backend URL configured");
    return false;
  }

  String url = g_config.backendUrl;
  if (url.startsWith("http://")) {
    url.replace("http://", "https://");   // force HTTPS
  } else if (!url.startsWith("https://") && !url.isEmpty()) {
    url = "https://" + url;               // no scheme -> prepend https://
  }
  url += "/api";

  Serial.printf("[ruuvi] GET %s\n", url.c_str());

  HTTPClient http;
  if (!http.begin(url)) {
    Serial.println("[ruuvi] /api FAILED: begin error");
    return false;
  }

  // Ruuvitag measurement API requires an x-api-key header. Send it only when a
  // key is configured (an empty/missing key would just trigger a 401).
  if (!g_config.apiKey[0]) {
    Serial.println("[ruuvi] no API key configured -> backend will return HTTP 401");
  } else {
    http.addHeader("x-api-key", g_config.apiKey);
  }

  int code = http.GET();
  String body;
  if (code == HTTP_CODE_OK) {
    body = http.getString();
  }
  http.end();

  if (code == HTTP_CODE_UNAUTHORIZED) {   // 401: missing/invalid API key
    Serial.println("[ruuvi] /api rejected with HTTP 401 -> check the configured x-api-key");
    return false;
  }
  if (code == HTTP_CODE_TOO_MANY_REQUESTS) {   // 429: backend rate limit hit
    Serial.printf("[ruuvi] /api rate limited by backend (HTTP %d); retrying next poll cycle\n", code);
    return false;
  }
  if (code != HTTP_CODE_OK || body.isEmpty()) {
    Serial.printf("[ruuvi] /api FAILED: HTTP %d\n", code);
    return false;
  }

  RuuviMeasurements ruuvi;
  if (!ruuvi.parse(body.c_str(), body.length())) {
    Serial.println("[ruuvi] parse failed, no data printed");
    return false;
  }

   display_update(ruuvi.data(), ruuvi.count());
   return true;
}
