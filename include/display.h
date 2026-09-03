#pragma once

#include <stdint.h>

#include "RuuviMeasurement.h"

// Initialize the e-paper driver, framebuffer and landscape rotation WITHOUT
// driving the physical panel. Safe to call on every boot (including after a
// deep-sleep reboot). Does not clear or render anything; only prepares RAM.
void display_framebuffer_init();

// Drive the physical panel blank once at boot (OK path only): powers on, pushes
// the all-white framebuffer and powers off so any previous content is removed.
void display_clear_panel();

// Render all available RuuviTag measurements onto the screen and push to the panel.
// Called after a successful /api poll in loop().
void display_update(const RuuviMeasurement* tags, uint8_t count);

// Draw an error/status message at the bottom-left of the panel (in its own band
// below the tag data) and drive the physical panel. Called on every /api failure
// path in main.cpp so a failed fetch still shows the retained tag data plus why it
// failed. Does NOT clear existing content; only adds the status line into the band.
void display_show_error(const char* message);
