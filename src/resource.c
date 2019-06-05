#include "resource.h"
#include "global.h"
#include "util.h"

#include <stdlib.h>
#include <osbind.h>

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

short init_resource()
{
	if (! rsrc_load(RESOURCENAME))
	{
		form_alert(1, "[3][RSC file not found!][Exit]");
		exit(-1);
	}
	return 0;
}

void free_resource()
{
	rsrc_free();
}
