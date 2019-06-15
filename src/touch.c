
#include "touch.h"
#include <gem.h>

static short x_offset;
static short y_offset;
static float sx;
static float sy;
extern short work_out[57];

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void)
{
  short width=work_out[0];
  short height=work_out[1];
  
  if (width==639)
    {
      x_offset=64;
      sx=1.0;
    }
  else if (width==319)
    {
      x_offset=0;
      sx=1.6;
    }
  else
    {
      x_offset=0;
      sx=1.0;
    }

  if (height==479)
    {
      height-=18;
    }
  else if (height==399)
    {
      height-=18;
    }
  else if (height==199)
    {
      height-=10;
    }

  sy=512/height;
}

/**
 * touch_main() - Main loop for touch screen
 * mx - mouse X from multi()
 * my - mouse Y from multi()
 */
void touch_main(short mx, short my)
{
  padPt coord;
  if (mx<x_offset)
    return;

  coord.x=(mx-x_offset)*sx;
  coord.y=(my)*sy;

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
