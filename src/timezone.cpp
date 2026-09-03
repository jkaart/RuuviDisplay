#include "timezone.h"

#include <Timezone.h> // NOLINT(cppcoreguidelines-avoid-magic-numbers)

namespace
{

  bool fillTmFromEpoch(time_t epoch, struct tm *outTm)
  {
    if (outTm == nullptr)
      return false;
    gmtime_r(&epoch, outTm);
    return true;
  }

  // Build the Timezone object from a ZoneEntry's two plain rules. The abbrev is
  // informational only and never affects computation.
  Timezone makeTimezone(const ZoneEntry *z)
  {
    return Timezone(
        TimeChangeRule(z->dst.abbrev, z->dst.week, z->dst.dow, z->dst.month, z->dst.hour, z->dst.offset_min),
        TimeChangeRule(z->std.abbrev, z->std.week, z->std.dow, z->std.month, z->std.hour, z->std.offset_min));
  }

} // namespace

// UTC epoch-seconds -> LOCAL wall-clock time for an IANA-style zone name.
// Declared in include/timezone.h; defined at global scope so the definition is
// exported and linkable from main.cpp/display.cpp (unlike the static helpers
// above, which stay internal to this translation unit).
bool utcToLocal(const char *zoneName, time_t utcEpochSeconds, struct tm *outTm)
{
  if (zoneName == nullptr || outTm == nullptr)
    return false;
  const ZoneEntry *z = tzLookup(zoneName);
  if (z == nullptr)
    return false;

  Timezone tz = makeTimezone(z);
  time_t localEpoch = tz.toLocal(utcEpochSeconds); // UTC + DST offset, in seconds
  return fillTmFromEpoch(localEpoch, outTm);       // LOCAL broken-down time
}
