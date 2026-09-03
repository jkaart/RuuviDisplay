#include "display.h"

#include "timezone.h" // UTC epoch -> Europe/Helsinki local time (DST-aware)

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

// Three fixed panels on a landscape 960x758 screen. Each panel shows one tag:
// its icon column (left) and the numeric value + unit text beside it.
static const int PANEL_CX[3] = {160, 480, 800};

// Icons sit in the left portion of each panel. The numeric value + unit text is
// right-aligned to each panel's right edge, so measurements occupy the right side
// while icons stay on the left. Timestamp line sits near the bottom-left.
// Layout derives from each panel's geometry (col_left = cx - HW) so all three
// tags share an identical structure and content stays inside its own panel.
static const int ICON_WIDTH = 60; // icons are 60x60 pixels
static const int ICON_HEIGHT = ICON_WIDTH;

static const int HW = 160;        // half panel width -> panels are exactly 320 wide, tiling [0,320]/[320,640]/[640,960] across the full 960px screen
static const int ICON_MARGIN = 5; // icon's margin from the panel edge (both sides)
static const int GAP = 5;         // gap between icon and value+unit text (= icon's right margin)

static const int TS_Y = 448;     // timestamp line, relative to top of screen
static const int SCREEN_W = 960; // landscape framebuffer width (ED047TC1)
static const int SCREEN_H = 540; // landscape framebuffer height

static const int ERROR_BAND_Y_GAP = 4;                  // error line small gap
static const int ERROR_BAND_Y = 530 - ERROR_BAND_Y_GAP; // status/error line sits near the bottom edge, above a small margin

// Number of tag panels shown on the display (must match PANEL_CX size).
#define PANEL_COUNT 3

static void draw_panel(uint8_t *fb, int cx, const RuuviMeasurement &m)
{
  int icon_x = (cx - HW) + ICON_MARGIN;
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
        .width = ICON_WIDTH,
        .height = ICON_HEIGHT,
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

    int text_x = cx + HW - GAP; // right edge of this panel - small gap -> text ends here, icons remain on the left
    int text_y = unit_y[i] + 40;
    epd_write_string(&OpenSans16B, buf, &text_x, &text_y, fb, &font_props);
  }
}

void display_update(const RuuviMeasurement *tags, uint8_t count)
{
  const int panels = (count < PANEL_COUNT) ? count : PANEL_COUNT;

  for (int i = 0; i < panels; ++i)
  {
    const RuuviMeasurement &m = tags[i];
    int cx = PANEL_CX[i];

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
    utcToLocal("Europe/Helsinki", ts, &tmv); // UTC epoch -> Helsinki local (DST-aware)
    strftime(time_buf, sizeof(time_buf), "%d/%m/%y %H:%M:%S", &tmv);

    EpdFontProperties ts_props = epd_font_properties_default();
    int ts_x = (cx - HW) + ICON_MARGIN; // timestamp sits at the icon column
    int ts_y = TS_Y;
    epd_write_string(&OpenSans12B, time_buf, &ts_x, &ts_y, g_fb, &ts_props);
  }

  // Erase the bottom band so any error line from a previous failed cycle is gone.
  // Tags are drawn above ERROR_BAND_Y, so this only clears where the status line lives;
  // it does not touch the retained tag data. Without it e-paper's persistent pixels would
  // keep showing an old error message on every subsequent successful render.
  epd_fill_rect(EpdRect{.x = 0, .y = ERROR_BAND_Y, .width = SCREEN_W, .height = SCREEN_H - ERROR_BAND_Y}, 0xFF, g_fb);

  // Power on FIRST so the panel is driven during data transfer, then off.
  // Without this the DC/CLK pulses are sent while VDD_IO is unpowered and the
  // physical panel never updates (tags never render).
  epd_poweron();
  epd_hl_update_screen(&g_hl, MODE_GC16, 0);
  epd_poweroff();
  epd_deinit();
}

// Draw an error/status message at the bottom-left of the panel and drive it. The tag
// data rendered by display_update() is preserved (e-paper retains its pixels), so a
// failed fetch shows the latest tags plus this line. Only the status text is added; no
// existing content is erased here.
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

  EpdFontProperties props = epd_font_properties_default();
  props.flags = EPD_DRAW_ALIGN_LEFT; // flush to the left edge of the screen
  int x = 2;                         // small margin so glyphs are not clipped at x=0
  int y = ERROR_BAND_Y;              // vertically centered in the bottom band

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
