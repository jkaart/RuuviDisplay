#pragma once

#include <cstdint>

namespace display_layout {

// --- Screen / panel geometry (landscape ED047TC1 framebuffer, 960x540) -------
// int16_t (not uint8_t): 960/540/448 exceed the 0..255 range of uint8_t and would
// be truncated, moving every coordinate.
constexpr int16_t DISPLAY_WIDTH   = 960;   // SCREEN_W
constexpr int16_t DISPLAY_HEIGHT  = 540;   // SCREEN_H
constexpr uint8_t PANEL_COUNT     = 3;

// Each tag panel is 320 wide; PANEL_CX[i] is its left edge and PANEL_HALF_WIDTH
// (the original HW) is the half-width used to mirror icon/text geometry around it.
constexpr int16_t PANEL_HALF_WIDTH = 160;
constexpr int16_t PANEL_CX[3] = {160, 480, 800};

// Icons and the gap between an icon and its value+unit text.
constexpr uint8_t ICON_WIDTH    = 60;
constexpr uint8_t ICON_HEIGHT   = ICON_WIDTH;
constexpr int16_t ICON_MARGIN_X = 5;
constexpr int16_t GAP_ICON_TEXT = 5;

// Timestamp line, relative to the top of the screen.
constexpr int16_t TS_Y_OFFSET = 448;

// Status/error band at the bottom of the screen and the "Last updated" row above it.
constexpr int16_t ERROR_BAND_Y_GAP    = 4;
constexpr int16_t LAST_UPDATED_GAP    = 18;
constexpr int16_t ERROR_BAND_Y        = 530 - ERROR_BAND_Y_GAP;   // = 526
constexpr int16_t LAST_UPDATED_ROW_Y  = ERROR_BAND_Y - LAST_UPDATED_GAP; // = 508

} // namespace display_layout
