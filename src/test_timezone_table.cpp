// Portable (host) unit tests for the built-in timezone zone table.
//
// Restores the coverage removed from test/test_timezone.cpp (deleted in the
// commit that added the NTP-based "Last updated" row). These tests run on any
// host C++17 toolchain with NO Arduino/ESP32 headers, so they are suitable for
// CI and local development.
//
// They link only against src/timezone_table.cpp (the Arduino-free zone table),
// NOT against src/timezone.cpp (whose utcToLocal() depends on the Arduino
// Timezone library and cannot be built natively). See the testenv SRC_FILTER in
// platformio.ini.
#include "timezone.h"

#include <cstdio>   // printf
#include <cstdlib>  // exit
#include <cstring>  // strcmp
#include <string>   // std::string for test messages

// Real-world standard-time UTC offset (minutes east of UTC) and the correct
// DST widening for each built-in zone, taken from the IANA tz database.
static const struct {
    const char* name;
    int stdOffsetMin;
    int dstOffsetMin;
    bool hasDst;
} kExpectedZones[] = {
    {"Europe/Helsinki",      +120, +180, true},  // EET / EEST
    {"Europe/London",            0,  +60, true},  // GMT / BST
    {"America/New_York",      -300, -240, true},  // EST / EDT
    {"America/Los_Angeles",   -480, -420, true},  // PST / PDT
    {"Asia/Tokyo",             +540, +540, false}, // JST, no DST
    {"Australia/Sydney",       +600, +660, true},  // AEST / AEDT
};

static int g_failures = 0;
static void check(bool cond, const char* msg)
{
    if (cond)
    {
        printf("ok   %s\n", msg);
    }
    else
    {
        ++g_failures;
        printf("FAIL %s\n", msg);
    }
}

// --- Table correctness: each zone's stored offsets must match the real-world
//     UTC offset of its standard-time rule, and DST rules must widen that
//     offset (or equal it for zones without daylight time). ---
static void testTableOffsets()
{
    for (const auto& c : kExpectedZones)
    {
        const ZoneEntry* z = tzLookup(c.name);
        check(z != nullptr && strcmp(z->name, c.name) == 0,
              (std::string("lookup + name match for ") + c.name).c_str());
        if (!z)
            continue;
        check(z->std.offset_min == c.stdOffsetMin,
              (std::string("std offset for ") + c.name).c_str());
        check(z->dst.offset_min == c.dstOffsetMin,
              (std::string("dst offset for ") + c.name).c_str());
        if (c.hasDst)
        {
            check(z->dst.offset_min > z->std.offset_min,
                  (std::string("DST widens offset for ") + c.name).c_str());
        }
        else
        {
            check(z->dst.offset_min == z->std.offset_min,
                  (std::string("no DST for ") + c.name).c_str());
        }
    }
}

// --- The table must contain every zone the firmware relies on. ---
static void testTableCompleteness()
{
    check(kNumTimezones >= 6, "table has at least the expected zones");
    check(tzLookup("Europe/Helsinki") != nullptr, "Europe/Helsinki (firmware default) is present");
}

// --- Unknown zone names must NOT be silently defaulted: lookup returns nullptr. ---
static void testUnknownZone()
{
    check(tzLookup("Mars/Phobos") == nullptr, "unknown zone returns nullptr");
    check(tzLookup("Not/AZone") == nullptr, "another unknown zone returns nullptr");
    check(tzLookup("") == nullptr, "empty zone name returns nullptr");
    check(tzLookup(nullptr) == nullptr, "null zone name returns nullptr");
}

int main()
{
    testTableOffsets();
    testTableCompleteness();
    testUnknownZone();
    printf("\n%d failure(s)\n", g_failures);
    return g_failures ? 1 : 0;
}
