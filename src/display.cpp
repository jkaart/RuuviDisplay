#include "display.h"

#include <epd_highlevel.h>   // transitively includes epidy.h (EpdRect, EpdFontProperties, EPD_DRAW_ALIGN_*, epd_fill_rect, ...)

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
static uint8_t* g_fb = nullptr;

// Three fixed panels on a landscape 960x758 screen. Each panel shows one tag:
// its icon column (left), unit labels, values and the measurement timestamp.
static const int PANEL_CX[3] = {160, 480, 800};

// Icon column sits in the left third of each panel; value labels sit to the
// right of center. Timestamp line sits near the bottom of the panel.
static const int ICON_X = -130;   // relative to panel center (icon width is 60)
static const int UNIT_X = 55;     // relative to panel center (right-aligned units)
static const int TS_Y = 478;      // timestamp line, relative to top of screen

// Number of tag panels shown on the display. Must match PANEL_CX / UNIT_LABELS size.
#define PANEL_COUNT 3

// Unit labels shown next to each icon.
static const char* UNIT_LABELS[4] = {"°C", "%RH", "hPa", "V"};

static void draw_panel(uint8_t* fb, int cx)
{
  EpdRect panel_rect = {
      .x = cx - 150,
      .y = 10,
      .width = 300,
      .height = 680,
  };
  epd_fill_rect(panel_rect, 0xFF, fb);

  int icon_x = cx + ICON_X;
  int unit_x = cx + UNIT_X;   // stable target for RIGHT-aligned text
  int unit_y[4] = {150, 218, 286, 354};

  for (int i = 0; i < 4; ++i)
  {
    const uint8_t* icon_data = nullptr;
    switch (i)
    {
    case 0: icon_data = temp_img_data; break;
    case 1: icon_data = hum_img_data; break;
    case 2: icon_data = pres_img_data; break;
    default: icon_data = batt_img_data; break;
    }

    EpdRect icon_rect = {
        .x = icon_x,
        .y = unit_y[i],
        .width = 60,
        .height = 60,
    };
    epd_copy_to_framebuffer(icon_rect, icon_data, fb);

    char label[8];
    snprintf(label, sizeof(label), "%s", UNIT_LABELS[i]);
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_RIGHT;
    epd_write_string(&OpenSans16B, label, &unit_x, &unit_y[i], fb, &font_props);
  }
}

void display_update(const RuuviMeasurement* tags, uint8_t count)
{
  const int panels = (count < PANEL_COUNT) ? count : PANEL_COUNT;

  for (int i = 0; i < panels; ++i)
  {
    const RuuviMeasurement& m = tags[i];
    int cx = PANEL_CX[i];

    EpdFontProperties name_props = epd_font_properties_default();
    name_props.flags = EPD_DRAW_ALIGN_CENTER;
    int name_y = 68;   // relative to top of screen (above first icon row)
    epd_write_string(&OpenSans24B, m.name, &cx, &name_y, g_fb, &name_props);

    draw_panel(g_fb, cx);

    char time_buf[16];
    struct tm tmv;
    time_t ts = (time_t)m.timestamp;
    gmtime_r(&ts, &tmv);
    strftime(time_buf, sizeof(time_buf), "%d/%m/%y %H:%M:%S", &tmv);

    EpdFontProperties ts_props = epd_font_properties_default();
    int ts_x = cx + ICON_X;   // timestamp sits at the icon column (panel center shifted left)
    int ts_y = TS_Y;
    epd_write_string(&OpenSans12B, time_buf, &ts_x, &ts_y, g_fb, &ts_props);
  }

  for (int i = panels; i < PANEL_COUNT; ++i)
  {
    EpdRect panel_rect = {
        .x = PANEL_CX[i] - 150,
        .y = 10,
        .width = 300,
        .height = 680,
    };
    epd_fill_rect(panel_rect, 0xFF, g_fb);
  }

  // Power on FIRST so the panel is driven during data transfer, then off.
  // Without this the DC/CLK pulses are sent while VDD_IO is unpowered and the
  // physical panel never updates (tags never render).
  epd_poweron();
  epd_hl_update_screen(&g_hl, MODE_GC16, 0);
  epd_poweroff();
}

void display_init()
{
  epd_init(&epd_board_lilygo_t5_47, &ED047TC1, EPD_OPTIONS_DEFAULT);
  g_hl = epd_hl_init(WAVEFORM);
  epd_set_rotation(EPD_ROT_LANDSCAPE);
  g_fb = epd_hl_get_framebuffer(&g_hl);
  epd_hl_set_all_white(&g_hl);

  // Power on FIRST so the panel is driven during data transfer, then off.
  // Without this the DC/CLK pulses are sent while VDD_IO is unpowered and the
  // physical panel never updates (display stays uncleared). Same sequence as
  // display_update().
  epd_poweron();
  epd_clear();
  //epd_hl_update_screen(&g_hl, MODE_GC16, 0);
  epd_poweroff();
}
