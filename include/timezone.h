#pragma once

#include <time.h>   // time_t, struct tm, gmtime_r

// Convert a UTC epoch-seconds timestamp to local Helsinki wall-clock time,
// applying Finnish daylight-saving rules (EET = UTC+2 in winter, EEST = UTC+3
// during summer). Fills *outTm with the broken-down LOCAL time. Returns true on
// success; returns false if outTm is null.

bool utcToLocalHelsinki(time_t utcEpochSeconds, struct tm* outTm);
