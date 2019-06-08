#include <gem.h>
#include <osbind.h>

#include "util.h"
#include "global.h"

extern short vdi_handle;

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

void init_userdef(void);

short init_util(void)
{
	return 0;
}

short free_util(void)
{
	return 0;
}


static short handle;
static USERBLK menu_blk;

#ifndef __GNUC__
/****************************************************************/
/* check if rectangles overlap                                  */
/****************************************************************/
short rc_intersect(register GRECT *r1, register GRECT *r2)
{
  register short w_i;
  register short h_i;

  w_i = (MIN(r1->g_x + r1->g_w, r2->g_x + r2->g_w) - MAX(r1->g_x, r2->g_x));
  h_i = (MIN(r1->g_y + r1->g_h, r2->g_y + r2->g_h) - MAX(r1->g_y, r2->g_y));

  return ((w_i > 0) && (h_i > 0));
}
#endif /* __GNUC__ */


short open_vwork(short work_out[])
{
	int i;

	for (i = 0; i < 10; work_in[i++] = 1);
	work_in[10] = 2;
	v_opnvwk(work_in, &vdi_handle, work_out);

	return vdi_handle;
}

void set_clipping(short handle, short x, short y, short w, short h, short on)
{
	short clip[4];

	clip[0] = x;
	clip[1] = y;
	clip[2] = clip[0] + w - 1;
	clip[3] = clip[1] + h - 1;

	vs_clip (handle, on, clip);
}

void fix_menu(OBJECT *tree)
{
    short i = -1;

    do {
        i++;
        if (tree[i].ob_type == G_STRING)
        {
            if ((tree[i].ob_state & OS_DISABLED) && (tree[i].ob_spec.free_string[0] == '-'))
            {
                tree[i].ob_type = (tree[i].ob_type << 8) + G_USERDEF;
                tree[i].ob_spec.userblk = &menu_blk;
            }
        }
    }
    while (!(tree[i].ob_flags & OF_LASTOB));
}

/*
 * Men<81>-Tuning: Trennlinien als Linie (f<81>r Hauptmen<81>s)
 * (ST-Computer 3/92, Seite 87)
 */
static short draw_menuline(PARMBLK *p)
{
    short pxy[4];

    vs_clip(handle, 0, pxy);

    pxy[0] = p->pb_x;
    pxy[1] = p->pb_y + (p->pb_h / 2) - 1;
    pxy[2] = p->pb_x + p->pb_w - 1;
    pxy[3] = p->pb_y + (p->pb_h / 2);

    vsf_interior(handle, FIS_PATTERN);
    vsf_style(handle, 4);
    vsf_color(handle, G_BLACK);
    vr_recfl(handle, pxy);

    return 0;
}

void init_userdef(void)
{
	handle = open_vwork(work_out);
    menu_blk.ub_parm = 0;
    menu_blk.ub_code = draw_menuline;
    v_clsvwk(handle);
}


