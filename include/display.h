#pragma once

#include <stdint.h>
#include <ctime>   // time_t: NTP epoch captured at render time, shown in "Last updated"

#include "RuuviMeasurement.h"

// NTP epoch (seconds since 1970) captured just before each render by main.cpp and
// read here to draw the "Last updated" row. Set to 0 when no time is available yet.
extern time_t g_renderEpoch;

// Local 18650 cell voltage (ADC pin 36), read by main.cpp before each render and
// drawn at the bottom of the panel. 0 until the first successful read.
extern double g_localBatteryVolts;

// Initialize the e-paper driver, framebuffer and landscape rotation WITHOUT
// driving the physical panel. Safe to call on every boot (including after a
// deep-sleep reboot). Does not clear or render anything; only prepares RAM.
void display_framebuffer_init();

// Drive the physical panel blank once at boot (OK path only): powers on, pushes
// the all-white framebuffer and powers off so any previous content is removed.
void display_clear_panel();

// Render all available RuuviTag measurements onto the screen and push to the panel.
// Called after a successful /api poll in loop(). Also draws the "Last updated" row
// above the status band and clears the band so a stale error line disappears.
void display_update(const RuuviMeasurement* tags, uint8_t count);

// Draw an error/status message at the bottom-left of the panel (in its own band
// below the tag data) plus the "Last updated" row above it, and drive the physical
// panel. Called on every /api failure path in main.cpp so a failed fetch still shows
// the retained tag data plus why it failed. The status band is cleared first so a
// stale error line is removed; g_renderEpoch (set by main before the call) is shown
// in the "Last updated" row.
void display_show_error(const char* message);

// Draw the bottom "Last updated" row (OK path): date + local time + "Last updated".
void draw_last_updated_row(uint8_t *fb);
