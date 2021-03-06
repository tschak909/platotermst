/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * screen.h - Display output functions
 */

#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include "protocol.h"
#include "window.h"

#define FGBG_FOREGROUND 0
#define FGBG_BACKGROUND 1

/**
 * Restore VDI palette
 */
void screen_restore_vdi_palette(void);

struct PLATOTermWindowData
{
  padByte* platoData;
  short platoLen;
};

/**
 * screen_update_status(void) - Update status bar
 */
void screen_update_status(void);

/**
 * screen_init() - Set up the screen
 */
void screen_init(void);

/**
 * screen_x() - Get screen X coordinates
 */
short screen_x(short x);

/**
 * screen_y() - Get screen Y coordinates
 */
short screen_y(short y);

/**
 * screen_strndup(ch, count) - duplicate character data.
 */
char* screen_strndup(unsigned char* ch, unsigned char count);

/**
 * screen_load_driver()
 * Load the TGI driver
 */
void screen_load_driver(void);

/**
 * screen_init_hook()
 * Called after tgi_init to set any special features, e.g. nmi trampolines.
 */
void screen_init_hook(void);

/**
 * screen_cycle_foreground()
 * Go to the next foreground color in palette
 */
void screen_cycle_foreground(void);

/**
 * screen_cycle_background()
 * Go to the next background color in palette
 */
void screen_cycle_background(void);

/**
 * screen_cycle_border()
 * Go to the next border color in palette
 */
void screen_cycle_border(void);

/**
 * screen_update_colors() - Set the terminal colors
 */
void screen_update_colors(void);

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void);

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void);

/**
 * screen_remap_palette(void)
 * Remap the screen palette
 */
void screen_remap_palette(void);

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void);

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2);

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord);

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2);

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count);

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar);

/**
 * screen_done()
 * Close down TGI
 */
void screen_done(void);

/**
 * screen_redraw()
 */
void screen_redraw(void);

/**
 * get screen color for mono displays
 */
short screen_color_mono(padRGB* theColor);

/**
 * return screen color index
 */
short screen_color(padRGB* theColor);

/**
 * Set foreground color
 */
void screen_foreground(padRGB* theColor);

/**
 * Set background color
 */
void screen_background(padRGB* theColor);

/**
 * Paint
 */
void screen_paint(padPt* Coord);

void screen_palette_dump(void);

/**
 * Show ready prompt
 */
void screen_show_ready(void);

/**
 * Window draw callback
 */
void screen_draw(struct window* wi, short x, short y, short w, short h);

/**
 * Window delete callback
 */
void screen_delete(struct window* wi);

#endif /* SCREEN_H */
