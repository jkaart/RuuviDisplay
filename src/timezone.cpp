#include "timezone.h"

#include <Timezone.h>   // jchristensen/Timezone (depends on PaulStoffregen Time)

// Europe/Helsinki rules. The library expresses each transition as the local
// wall-clock moment it occurs plus the resulting UTC offset in minutes:
//   summer / daylight time  EEST = UTC+3, begins last Sunday of March at 02:00
//   standard time           EET  = UTC+2, begins last Sunday of October at 03:00
static const TimeChangeRule g_dstRule = TimeChangeRule("EEST", Last, Sun, Mar, 2, +180);
static const TimeChangeRule g_stdRule = TimeChangeRule("EET",  Last, Sun, Oct, 3, +120);

// Single timezone instance is sufficient: Finland's rules never change at runtime.
static Timezone g_helsinki(g_dstRule, g_stdRule);

bool utcToLocalHelsinki(time_t utcEpochSeconds, struct tm* outTm)
{
  if (!outTm) {
    return false;
  }
  // toLocal() returns the epoch shifted by the correct (DST-aware) offset for
  // this timestamp's year. gmtime_r then turns that local epoch into wall-clock
  // fields, so %H:%M reflect Helsinki time (+2/+3 h), not UTC.
  time_t localEpoch = g_helsinki.toLocal(utcEpochSeconds);
  gmtime_r(&localEpoch, outTm);
  return true;
}
