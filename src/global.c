#include <stdint.h>
#include "global.h"
#include "window.h"

//#define DEBUG
#ifdef DEBUG
/*
 * Since writing directly somewhere to the screen would distort GEM, the escape sequences in debug_printf()
 * position the cursor on line 30, column 0, clear to end of line and write the debug message.
 * Make sure you don't add a newline after the message or the screen will be clobbered.
 * This way we have at least one single line to display diagnostic output.
 */
#define debug_printf(format, arg...) do { printf("\033Y\36 \33lDEBUG (%s): " format, __FUNCTION__, ##arg); } while (0)
#else
#define debug_printf(format, arg...) do { ; } while (0)
#endif /* DEBUG */


short vdi_handle;    		/* virtual workstation handle */

short work_in[11];   		/* Input to GSX parameter array */
short work_out[57];  		/* Output from GSX parameter array */

short ap_id;

short window_open_pos_x;
short window_open_pos_y;

short phys_handle;

short gl_wbox;
short gl_hbox;
short gl_hchar;
short gl_wchar;

OBJECT *gl_menu;

GRECT gl_desk;

void init_global(void)
{
	window_open_pos_x = 20;
	window_open_pos_y = 20;
}

void free_global(void)
{
}


