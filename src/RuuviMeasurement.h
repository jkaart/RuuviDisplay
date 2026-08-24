#pragma once

#include <stdint.h>
#include <string.h>

// One RuuviTag measurement as parsed from the backend /api JSON object.
struct RuuviMeasurement {
    float temperature = 0.0f;      // °C
    float humidity = 0.0f;         // %
    float pressure = 0.0f;         // Pa (raw value from backend)
    float batteryVoltage = 0.0f;   // V
    unsigned long timestamp = 0;   // epoch seconds
    char name[32] = {0};           // tag name (optional in JSON)
    char mac[18] = {0};            // "AA:BB:CC:DD:EE:FF"
};

// Parses a JSON array of measurement objects and keeps the latest object per MAC.
class RuuviMeasurements {
public:
    static const int kMaxTags = 32;

    bool parse(const char* json, size_t len);   // replace all stored data
    uint8_t printAll();                          // print every tag to Serial

private:
    RuuviMeasurement tags_[kMaxTags];
    uint8_t count_ = 0;
};
