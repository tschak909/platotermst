#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <stdbool.h>

#include "window.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

extern void init_global(void);
extern void free_global(void);

/*
 * GLOBAL VARIABLES
/*/

extern short phys_handle;		/* physical workstation handle */
extern short vdi_handle;		/* virtual workstation handle */
extern short contrl[12];
extern short shortin[128];
extern short ptsin[128];
extern short shortout[128];
extern short ptsout[128];		/* storage wasted for idiotic bindings */

extern short work_in[11];		/* Input to GSX parameter array */
extern short work_out[57];	/* Output from GSX parameter array */
extern short pxyarray[10];	/* input poshort array */

extern short ap_id;

extern short window_open_pos_x;	/* where to open new windows */
extern short window_open_pos_y;

extern short gl_wchar, gl_hchar, gl_wbox, gl_hbox;
extern OBJECT *gl_menu;
extern GRECT gl_desk;

extern bool quit;
#endif


