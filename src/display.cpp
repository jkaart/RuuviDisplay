#include "display.h"
#include <Arduino.h>

#include "timezone.h" // UTC epoch -> local time (DST-aware)
#include "wifi_config.h" // WIFI_MANAGER_PARAM_TIMEZONE_DEFAULT (fallback zone)

#include <epd_highlevel.h> // transitively includes epidy.h (EpdRect, EpdFontProperties, EPD_DRAW_ALIGN_*, epd_fill_rect, ...)

#include "temp_img.h"
#include "hum_img.h"
#include "pres_img.h"
#include "batt_img.h"

#include "opensans24b.h"
#include "opensans16b.h"
#include "opensans12b.h"

#include <stdio.h>
#include <time.h>

// --- E-paper driver state (landscape, ED047TC1) -----------------------------
#define WAVEFORM EPD_BUILTIN_WAVEFORM

static EpdiyHighlevelState g_hl;
static uint8_t *g_fb = nullptr;

#include "display_layout.h" // panel/icon/gap/position layout constants (see include/)

static void draw_panel(uint8_t *fb, int cx, const RuuviMeasurement &m)
{
  int icon_x = (cx - display_layout::PANEL_HALF_WIDTH) + display_layout::ICON_MARGIN_X;
  int unit_y[4] = {150, 218, 286, 354};

  char buf[16];

  for (int i = 0; i < 4; ++i)
  {
    const uint8_t *icon_data = nullptr;
    switch (i)
    {
    case 0:
      icon_data = temp_img_data;
      break;
    case 1:
      icon_data = hum_img_data;
      break;
    case 2:
      icon_data = pres_img_data;
      break;
    default:
      icon_data = batt_img_data;
      break;
    }

    EpdRect icon_rect = {
        .x = icon_x,
        .y = unit_y[i],
        .width = display_layout::ICON_WIDTH,
        .height = display_layout::ICON_HEIGHT,
    };
    epd_copy_to_framebuffer(icon_rect, icon_data, fb);

    if (m.temperature == 0 && m.humidity == 0 && m.pressure == 0)
    {
      snprintf(buf, sizeof(buf), "%s", "--");
    }
    else if (i == 0)
    {
      snprintf(buf, sizeof(buf), "%.2f %s", m.temperature, "°C");
    }
    else if (i == 1)
    {
      snprintf(buf, sizeof(buf), "%.2f %s", m.humidity, "%RH");
    }
    else if (i == 2)
    {
      snprintf(buf, sizeof(buf), "%.1f %s", m.pressure / 100.0f, "hPa");
    }
    else
    {
      snprintf(buf, sizeof(buf), "%.3f %s", m.batteryVoltage, "V");
    }

    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_RIGHT; // value+unit right-aligned to the panel's right edge (icons stay left)

    int text_x = cx + display_layout::PANEL_HALF_WIDTH - display_layout::GAP_ICON_TEXT; // right edge of this panel - small gap -> text ends here, icons remain on the left
    int text_y = unit_y[i] + 40;
    epd_write_string(&OpenSans16B, buf, &text_x, &text_y, fb, &font_props);
  }
}

void display_update(const RuuviMeasurement *tags, uint8_t count)
{
  const int panels = (count < display_layout::PANEL_COUNT) ? count : display_layout::PANEL_COUNT;

  for (int i = 0; i < panels; ++i)
  {
    const RuuviMeasurement &m = tags[i];
    int cx = display_layout::PANEL_CX[i];

    // Tag names

    EpdFontProperties name_props = epd_font_properties_default();
    name_props.flags = EPD_DRAW_ALIGN_CENTER;
    int name_y = 68; // relative to top of screen (above first icon row)
    int name_x = cx; // separate from cx: epd_write_line mutates its *cursor_x arg as a side effect (~w/2 for ALIGN_CENTER), which would shift draw_panel(g_fb, cx, m) below.
    epd_write_string(&OpenSans24B, m.name, &name_x, &name_y, g_fb, &name_props);

    draw_panel(g_fb, cx, m);

    // Tag timestamps

    char time_buf[18];
    struct tm tmv;
    time_t ts = (time_t)m.timestamp;
    utcToLocal(tzEffectiveZone(g_timezone, WIFI_MANAGER_PARAM_TIMEZONE_DEFAULT), ts, &tmv); // UTC epoch -> local (DST-aware)
    strftime(time_buf, sizeof(time_buf), "%d/%m/%y %H:%M:%S", &tmv);

    EpdFontProperties ts_props = epd_font_properties_default();
    int ts_x = (cx - display_layout::PANEL_HALF_WIDTH) + display_layout::ICON_MARGIN_X; // timestamp sits at the icon column
    int ts_y = display_layout::TS_Y_OFFSET;
    epd_write_string(&OpenSans12B, time_buf, &ts_x, &ts_y, g_fb, &ts_props);
  }

  // Erase the status band so any error line from a previous failed cycle is gone.
  // The band spans from LAST_UPDATED_ROW_Y (the "Last updated" row) to the bottom edge,
  // so both the row and the error line are cleared; the retained tag data drawn above
  // stays intact. Without this e-paper's persistent pixels would keep showing an old
  // error message on every subsequent successful render.
  epd_fill_rect(EpdRect{.x = 0, .y = display_layout::LAST_UPDATED_ROW_Y, .width = display_layout::DISPLAY_WIDTH, .height = display_layout::DISPLAY_HEIGHT - display_layout::LAST_UPDATED_ROW_Y}, 0xFF, g_fb);

  // Draw the "Last updated" row (g_renderEpoch, set by main before this call).
  draw_last_updated_row(g_fb);

  // Power on FIRST so the panel is driven during data transfer, then off.
  // Without this the DC/CLK pulses are sent while VDD_IO is unpowered and the
  // physical panel never updates (tags never render).
  epd_poweron();
  epd_hl_update_screen(&g_hl, MODE_GC16, 0);
  epd_poweroff();
  epd_deinit();
}

// Draw the "Last updated" row just above the status/error band. Shows g_renderEpoch
// (set by main.cpp before each render) converted to local time. If no time is
// available yet (g_renderEpoch == 0) it draws "--". Only the row is added into the
// already-erased band; tag data above LAST_UPDATED_ROW_Y is never touched.
void draw_last_updated_row(uint8_t *fb)
{
  char buf[32];
  if (g_renderEpoch == 0)
  {
    snprintf(buf, sizeof(buf), "--");
  }
  else
  {
    struct tm tmv;
    char time_buf[20];
    utcToLocal(tzEffectiveZone(g_timezone, WIFI_MANAGER_PARAM_TIMEZONE_DEFAULT), g_renderEpoch, &tmv); // UTC epoch -> local (DST-aware)
    strftime(time_buf, sizeof(time_buf), "%d/%m/%y %H:%M:%S", &tmv);
    snprintf(buf, sizeof(buf), "Last updated: %s", time_buf);
  }

  EpdFontProperties props = epd_font_properties_default();
  props.flags = EPD_DRAW_ALIGN_LEFT; // flush to the left edge, aligned with the error line
  int x = 2;                         // small margin so glyphs are not clipped at x=0
  int y = display_layout::LAST_UPDATED_ROW_Y;

  epd_write_string(&OpenSans12B, buf, &x, &y, fb, &props);
}

// Draw an error/status message at the bottom-left of the panel and drive it. The tag
// data rendered by display_update() is preserved (e-paper retains its pixels), so a
// failed fetch shows the latest tags plus this line. The status band is cleared first
// (row + error area) so a stale error line is removed; the "Last updated" row is redrawn
// from g_renderEpoch (last successful update). Only the status text is added; no other
// content is erased here.
void display_show_error(const char *message)
{
  char msg[65] = {0};
  if (message && message[0])
  {
    snprintf(msg, sizeof(msg), "%s", message);
  }
  else
  {
    snprintf(msg, sizeof(msg), "--");
  }

  // Clear the status band (row + error area) so a stale error line from a previous
  // failed cycle is removed before we redraw.
  epd_fill_rect(EpdRect{.x = 0, .y = display_layout::LAST_UPDATED_ROW_Y, .width = display_layout::DISPLAY_WIDTH, .height = display_layout::DISPLAY_HEIGHT - display_layout::LAST_UPDATED_ROW_Y}, 0xFF, g_fb);

  // "Last updated" row (retains the last successful update time from g_renderEpoch).
  draw_last_updated_row(g_fb);

  // Error line below the row.
  EpdFontProperties props = epd_font_properties_default();
  props.flags = EPD_DRAW_ALIGN_LEFT; // flush to the left edge of the screen
  int x = 2;                         // small margin so glyphs are not clipped at x=0
  int y = display_layout::ERROR_BAND_Y;

  epd_write_string(&OpenSans12B, msg, &x, &y, g_fb, &props);

  // Drive the physical panel.
  epd_poweron();
  epd_hl_update_screen(&g_hl, MODE_GC16, 0);
  epd_poweroff();
}

void display_framebuffer_init()
{
  epd_init(&epd_board_lilygo_t5_47, &ED047TC1, EPD_OPTIONS_DEFAULT);
  g_hl = epd_hl_init(WAVEFORM);
  epd_set_rotation(EPD_ROT_LANDSCAPE);
  g_fb = epd_hl_get_framebuffer(&g_hl);
  epd_hl_set_all_white(&g_hl);
}

void display_clear_panel()
{
  // Power on FIRST so the panel is driven during data transfer, then off.
  // Without this the DC/CLK pulses are sent while VDD_IO is unpowered and the
  // physical panel never updates (display stays uncleared). Same sequence as
  // display_update(). Drives a blank framebuffer to remove any previous content.
  epd_poweron();
  epd_clear();
  epd_poweroff();
}
