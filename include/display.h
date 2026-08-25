#pragma once

#include <stdint.h>

#include "RuuviMeasurement.h"

// Initialize the e-paper driver, framebuffer and landscape rotation.
// Call this exactly once during setup(), before any rendering happens.
void display_init();

// Render all available RuuviTag measurements onto the screen and push to the panel.
// Called after a successful /api poll in loop().
void display_update(const RuuviMeasurement* tags, uint8_t count);
