
#include "touch.h"
#include <gem.h>

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void)
{
}

/**
 * touch_main() - Main loop for touch screen
 */
void touch_main(void)
{
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
  if (allow)
    graf_mouse(M_OFF,0x0L);
  else
    graf_mouse(M_ON,0x0L);
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
