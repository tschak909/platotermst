
#include "protocol.h"
#include "math.h"
#include "screen.h"
#include "terminal.h"
#include <gem.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <osbind.h>
#include <string.h>
#include <math.h>
#include "window.h"

short width, height;
unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
padRGB palette[16];
unsigned char FONT_SIZE_X;
unsigned char FONT_SIZE_Y;
unsigned short* scalex;
unsigned short* scaley;
unsigned char* font[];
short background_color_index=0;
short foreground_color_index=1;
padRGB background_rgb={0,0,0};
padRGB foreground_rgb={255,255,255};
unsigned char highestColorIndex=0;

extern padBool FastText; /* protocol.c */

extern unsigned char fontm23[];
extern unsigned short full_screen;
extern unsigned short window_x;
extern unsigned short window_y;
extern short appl_is_mono;

static char tmptxt[80];

struct window* screen_window;

extern short work_out[57];

#define VDI_COLOR_SCALE 3.91
#define PLATOTERMWINDOW_CLASS 0x7074726d // ptrm

/**
 * screen_strndup(ch, count) - duplicate character data.
 */
char* screen_strndup(unsigned char* ch, unsigned char count)
{
  char* ret=calloc((char)count,sizeof(char));

  if (ret==NULL)
    return NULL;

  memcpy(ret,ch,count);
  return ret;
  
}

/**
 * Window draw callback
 */
void screen_draw(struct window* wi, short x, short y, short w, short h)
{
  screen_window->clear(screen_window,x,y,w,h);
}

/**
 * Window delete callback
 */
void screen_delete(struct window* wi)
{
  // Pass to window.c to pass it to AES
  delete_window(wi);
}

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  width=work_out[0];
  height=work_out[1];
  screen_window=create_window(0,"PLATOTERM");
  screen_window->class=PLATOTERMWINDOW_CLASS;
  screen_window->draw=screen_draw;
  screen_window->del=screen_delete;

  if (width==639 && height==479)
    {
      // TT Med Res.
    }
  else if (width==639 && height==399)
    {
      // ST High res
    }
  else if (width==639 && height==199)
    {
      // ST Med res
    }
  else if (width==319 && height==199)
    {
      // ST low res
    }

  open_window(screen_window, 0, 0, width, height);
  do_redraw(screen_window,
	    screen_window->work.g_x,
	    screen_window->work.g_y,
	    screen_window->work.g_w,
	    screen_window->work.g_h);
}

/**
 * screen_load_driver()
 * Load the TGI driver
 */
void screen_load_driver(void)
{
}

/**
 * screen_init_hook()
 * Called after tgi_init to set any special features, e.g. nmi trampolines.
 */
void screen_init_hook(void)
{
}

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
}

/**
 * screen_remap_palette(void)
 * Remap the screen palette
 */
void screen_remap_palette(void)
{
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
}

/**
 * screen_char_bold_shift() - enlarge character for bold mode
 */
void screen_char_bold_shift(unsigned short* bold_char, unsigned short* ch)
{
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
}

/**
 * screen_done()
 * Close down TGI
 */
void screen_done(void)
{
}

/**
 * screen_redraw()
 */
void screen_redraw(void)
{
}

/**
 * Screen palette dump - remove when working
 */
void screen_palette_dump(void)
{
}

/**
 * Set foreground color
 */
void screen_foreground(padRGB* theColor)
{
}

/**
 * Set background color
 */
void screen_background(padRGB* theColor)
{
}

/**
 * screen_color_matching(theColor)
 */
short screen_color_matching(padRGB* theColor)
{
}

/**
 * get screen color for mono displays
 */
short screen_color_mono(padRGB* theColor)
{
}

/**
 * Set selected screen color (fg/bg)
 */
short screen_color(padRGB* theColor)
{
}

/**
 * paint
 */
void screen_paint(padPt* Coord)
{
}

