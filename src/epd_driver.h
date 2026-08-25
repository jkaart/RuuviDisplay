// Compatibility shim for epdiy >= 2.x.
//
// In epdiy 2.1.3 the driver-level declarations were consolidated into epidy.h
// and the standalone "epd_driver.h" header was removed. Vendored icon/font
// libraries under lib/ still #include "epd_driver.h"; this shim re-exports the
// current epidy.h so EpdRect, EpdFontProperties, EPD_DRAW_ALIGN_* and the
// epd_fill_rect / epd_copy_to_framebuffer / epd_write_string declarations stay
// available without modifying third-party sources.

#include <epdiy.h>
