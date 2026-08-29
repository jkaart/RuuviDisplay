#include "RuuviMeasurement.h"

#include "timezone.h"   // UTC epoch -> Europe/Helsinki local time (DST-aware)

#include <ArduinoJson.h>
#include <time.h>

bool RuuviMeasurements::parse(const char* json, size_t len)
{
  if (!json || !len) {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.printf("[ruuvi] parse FAILED: %s (%zu bytes)\n", error.c_str(), len);
    return false;
  }

  JsonArray arr = doc.as<JsonArray>();
  for (    JsonVariant v : arr) {
    RuuviMeasurement m;

    // Only objects carry named fields; skip anything else safely. JsonObject is the
    // only object subtype exposed by this build, so it guards the field access below.
    // Missing keys are created as defaults on a mutable variant and read back via the
    // generic as<T>() template -> no Number/String subtype type is needed in this build.
    if (!v.is<JsonObject>()) { continue; }

    m.temperature     = v["temperature"].as<float>();
    m.humidity        = v["humidity"].as<float>();
    m.pressure        = v["pressure"].as<float>();
    m.batteryVoltage  = v["batteryVoltage"].as<float>();
    m.timestamp       = v["timestamp"].as<unsigned long>();

    // JsonString is not available in this build, so guard string reads at runtime and
    // skip null/empty values (leaving the fields at their default nullptr).
    const char* name = v["name"].as<const char*>();
    if (name && name[0]) { strncpy(m.name, name, sizeof(m.name) - 1); }
    const char* mac   = v["mac"].as<const char*>();
    if (mac && mac[0]) { strncpy(m.mac, mac, sizeof(m.mac) - 1); }

    // Keep the latest object per MAC address.
    uint8_t idx = count_;
    for (uint8_t i = 0; i < count_; i++) {
      if (strcmp(tags_[i].mac, m.mac) == 0) { idx = i; break; }
    }
    tags_[idx] = m;
    if (idx >= count_) { count_++; }
  }
  return true;
}

uint8_t RuuviMeasurements::printAll()
{
  for (uint8_t i = 0; i < count_; i++) {
    const RuuviMeasurement& m = tags_[i];

    Serial.print("[ruuvi] ");
    if (m.name[0]) {
      Serial.printf("%s (%s)", m.name, m.mac);
    } else {
      Serial.printf("untitled (%s)", m.mac);
    }
    Serial.printf(" @ ");

    struct tm tm;
    time_t ts_time = m.timestamp;   // unsigned long -> time_t (long long on ESP32)
    utcToLocalHelsinki(ts_time, &tm);   // UTC epoch -> Helsinki local (DST-aware)
    char ts[17];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", &tm);
    Serial.print(ts);
    Serial.println();

    Serial.printf("    temperature: %.2f °C\n", m.temperature);
    Serial.printf("    humidity:   %.2f %%\n", m.humidity);
    // Backend reports pressure in Pa; display in hPa (divide by 100).
    Serial.printf("    pressure:  %.1f hPa\n", m.pressure / 100.0f);
    Serial.printf("    batteryVoltage: %.3f V\n", m.batteryVoltage);
  }
  return count_;
}
