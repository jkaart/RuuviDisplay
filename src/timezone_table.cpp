#include "timezone.h"

#include <cstring> // strcmp

// Built-in IANA-style zone table used by utcToLocal(). Kept in its own
// translation unit so the data can be compiled and unit-tested without the
// Arduino/ESP32 headers. Offsets are minutes east of UTC; positive = east.
// DST rules follow each zone's actual daylight-saving schedule:
//   dst  = rule that begins summer/daylight time (or a single std rule if none)
//   std  = rule that begins standard time
//
// jchristensen TimeChangeRule field values used here:
//   week: First=1 Second=2 Third=3 Fourth=4 Last=5
//   dow : Sun=1 Mon=2 Tue=3 Wed=4 Thu=5 Fri=6 Sat=7

TzRule rule(const char *abbrev, int8_t week, int8_t dow, int8_t month, int8_t hour, int offset_min)
{
  return TzRule{abbrev, week, dow, month, hour, offset_min};
}

const ZoneEntry kTimezones[] = {
    {"Europe/Helsinki", rule("EEST", 5, 1, 3, 2, +180), rule("EET", 1, 1, 10, 3, +120)},
    {"Europe/London", rule("BST", 5, 1, 3, 2, +60), rule("GMT", 1, 1, 10, 2, +0)},
    {"America/New_York", rule("EDT", 2, 1, 3, 2, -240), rule("EST", 1, 1, 11, 2, -300)},
    {"America/Los_Angeles", rule("PDT", 2, 1, 3, 2, -420), rule("PST", 1, 1, 11, 2, -480)},
    {"Asia/Tokyo", rule("JST", 1, 1, 1, 0, +540), rule("JST", 1, 1, 1, 0, +540)},
    {"Australia/Sydney", rule("AEDT", 1, 1, 10, 2, +660), rule("AEST", 1, 1, 4, 2, +600)},
};

const size_t kNumTimezones = sizeof(kTimezones) / sizeof(kTimezones[0]);

// Return the ZoneEntry matching an IANA-style name, or nullptr if unknown.
const ZoneEntry *tzLookup(const char *zoneName)
{
  if (zoneName == nullptr)
    return nullptr;
  for (size_t i = 0; i < kNumTimezones; ++i)
  {
    if (strcmp(kTimezones[i].name, zoneName) == 0)
      return &kTimezones[i];
  }
  return nullptr;
}
