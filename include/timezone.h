#pragma once

#include <time.h>   // time_t, struct tm
#include <cstdint>  // int8_t

// Plain (Arduino-independent) representation of a repeating DST rule set. The
// offset is minutes from UTC; positive = east of the prime meridian. This type
// has no dependency on the Arduino Time libraries so the zone table can be unit
// tested on any host compiler.
struct TzRule {
  const char* abbrev;   // human-readable name, e.g. "EEST" (informational only)
  int8_t week;          // Last, First, Second, Third, Fourth
  int8_t dow;           // day of week: 1=Sun .. 7=Sat
  int8_t month;         // 1=Jan .. 12=Dec
  int8_t hour;          // local wall-clock time the transition occurs at
  int offset_min;       // UTC offset in minutes (positive east)
};

// A timezone's two repeating rules: daylight/summer-time start and standard-
// time start. For zones that do not observe daylight time both rules are equal.
struct ZoneEntry {
  const char* name;
  TzRule dst;   // daylight / summer-time start (or single std rule)
  TzRule std;   // standard-time start
};

// Built-in zone table. Defined in src/timezone_table.cpp; see include/timezone.h
// for the Arduino-free type used by utcToLocal and its unit tests.
extern const ZoneEntry kTimezones[];
extern const size_t kNumTimezones;

// Convert a UTC epoch-seconds timestamp to the LOCAL wall-clock time of an
// IANA-style timezone name, applying DST rules when that zone observes them.
// Fills *outTm with broken-down LOCAL time. Returns true on success; returns
// false if outTm is null or the zone name is not recognized by the built-in
// table (an unknown zone is intentionally NOT silently defaulted).
bool utcToLocal(const char* zoneName, time_t utcEpochSeconds, struct tm* outTm);

// Return the ZoneEntry matching an IANA-style name, or nullptr if unknown.
// Public so callers (and unit tests) can validate the table without building
// the Arduino Timezone dependency.
const ZoneEntry* tzLookup(const char* zoneName);
