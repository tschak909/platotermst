
#include "touch.h"
#include "window.h"
#include <gem.h>
#include <math.h>
#include <stdio.h>

extern struct window* screen_window;
extern short vxoff,yoff;

extern short work_out[57];

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void)
{
}

/**
 * touch_main() - Main loop for touch screen
 * mx - mouse X from multi()
 * my - mouse Y from multi()
 */
void touch_main(short mx, short my)
{
  padPt coord;
  short windowWidth=(work_out[0]>512 ? 512 : screen_window->work.g_w);
  short windowHeight=screen_window->work.g_h;

  coord.x = ((long)mx * PLATOSize.x) / windowWidth;
  coord.x-=vxoff;
  coord.y = (PLATOSize.y - 1) - (((long)my * PLATOSize.y) /
  				windowHeight);
  coord.y+=yoff;

  
  Touch(&coord);
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
}

/**
 * handle_mouse - Process mouse events and turn into scaled touch events
 */
void handle_mouse(void)
{
}

/**
 * touch_hide() - hide the mouse cursor
 */
void touch_hide(void)
{
}

/**
 * touch_done() - Stop the mouse driver
 */
void touch_done(void)
{
}
