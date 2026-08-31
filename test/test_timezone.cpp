// Portable (host) unit tests for include/timezone.h. No Arduino/ESP32 headers
// required; links only src/timezone_table.cpp so it runs on any C++ toolchain.
#include "timezone.h"

#include <cstdio>   // printf
#include <cstdlib>  // exit
#include <cstring>  // strcmp

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { ++failures; printf("FAIL %s\n", #cond); } else { printf("ok   %s\n", #cond); } } while (0)

// --- Table correctness: each zone's stored offsets must match the real-world
//     UTC offset of its standard-time rule, and DST rules must widen that
//     offset (or equal it for zones without daylight time). ---
static void testTableOffsets() {
  struct { const char* name; int stdOffsetMin; int dstOffsetMin; bool hasDst; } cases[] = {
      {"Europe/Helsinki",   +120, +180, true},
      {"Europe/London",     +0,   +60,  true},
      {"America/New_York",  -300, -240, true},
      {"America/Los_Angeles",-480, -420, true},
      {"Asia/Tokyo",        +540, +540, false},
      {"Australia/Sydney",  +600, +660, true},
  };
  for (const auto& c : cases) {
    const ZoneEntry* z = tzLookup(c.name);
    CHECK(z != nullptr && strcmp(z->name, c.name) == 0);
    if (!z) continue;
    CHECK(z->std.offset_min == c.stdOffsetMin);
    CHECK(z->dst.offset_min == c.dstOffsetMin);
    CHECK((c.hasDst ? z->dst.offset_min > z->std.offset_min : z->dst.offset_min == z->std.offset_min));
  }
}

// --- Unknown zone must NOT be silently defaulted: lookup returns nullptr. ---
static void testUnknownZone() {
  CHECK(tzLookup("Mars/Phobos") == nullptr);
  CHECK(tzLookup(nullptr) == nullptr);
}

int main() {
  testTableOffsets();
  testUnknownZone();
  printf("\n%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
