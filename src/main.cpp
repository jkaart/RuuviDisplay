#include <Arduino.h>

#include "wifi_config.h"

#include <WiFiManager.h>

// -- Callbacks ---------------------------------------------------------------
void configModeCallback(WiFiManager *myWiFiManager);
void saveConfigCallback();

static bool g_shouldReboot = false;

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("[wifi] Entered configuration mode");
  Serial.println("AP name: " + myWiFiManager->getConfigPortalSSID());
}

void saveConfigCallback()
{
  // Custom parameters were saved and a connection was established. Flag a
  // reboot so the ESP32 re-reads the new credentials from flash on next boot.
  g_shouldReboot = true;
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
}

void loop()
{
  // Idle for now. Future work: initialize the EPDiy display and run the sensor
  // update loop here (e.g. epd.init(); epd.updatePanel()).
}
