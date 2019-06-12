#include <stdbool.h>
#include <gem.h>
#include <string.h>
#include "protocol.h"
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"
#include "util.h"
#include "window.h"
#include "menu.h"
#include "resource.h"
#include "global.h"
#include "dialog.h"

unsigned char already_started=false;

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

static short msgbuff[8];    /* event message buffer */
static short mx;
static short my;            /* mouse x and y pos. */
static short butdown;       /* button state tested for, UP/DOWN */
static short ret;           /* dummy return variable */

struct dialog_handler* about_dialog;
struct dialog_handler* prefs_dialog;
struct dialog_handler* keys_dialog;
struct dialog_handler* micro_dialog;

bool quit;

int timer_cb(struct window *wi)
{
    if (wi->timer) (*wi->timer)(wi);

    return 0;
}

void multi(void)
{
    short event;
    short keystate;
    short keyreturn;
    long msec = 0;
    struct window *wi = NULL;

    do
    {
        event = evnt_multi(MU_MESAG /* | MU_BUTTON */ | MU_KEYBD | MU_TIMER,
                        1, 1, butdown,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        msgbuff, msec, &mx, &my, &ret, &keystate, &keyreturn, &ret);

        wind_update(true);

	if (event & MU_KEYBD)
	  {
	    keyboard_main(keyreturn,keystate);
	  }
	
        if (event & MU_MESAG)
        {
            wi = from_handle(msgbuff[3]);
            switch (msgbuff[0])
            {	      
                case WM_REDRAW:
                    do_redraw(wi, msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                    break;

                case WM_NEWTOP:
                case WM_ONTOP:
                case WM_TOPPED:
                    wind_set(wi->handle, WF_TOP, 0, 0, 0, 0);
                    wi->topped = true;
                    break;

                case WM_UNTOPPED:
                    wi->topped = false;
                    break;

                case WM_SIZED:
                /* case WM_MOVED: */
                /*     if (wi->size) wi->size(wi, msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]); */
                /*     break; */

                /* case WM_FULLED: */
                /*     if (wi->full) wi->full(wi); */
                /*     break; */

                /* case WM_CLOSED: */
                /*     if (wi->del) wi->del(wi); */
                /*     break; */

                /* case WM_ARROWED: */
                /*     switch (msgbuff[4]) */
                /*     { */
                /*         case WA_UPPAGE: */
                /*             wi->top -= wi->work.g_h / wi->y_fac; */
                /*             break; */

                /*         case WA_DNPAGE: */
                /*             wi->top += wi->work.g_h / wi->y_fac; */
                /*             break; */

                /*         case WA_UPLINE: */
                /*             wi->top--; */
                /*             break; */

                /*         case WA_DNLINE: */
                /*             wi->top++; */
                /*             break; */

                /*         case WA_LFPAGE: */
                /*             wi->left -= wi->doc_width - wi->work.g_w / wi->x_fac; */
                /*             break; */

                /*         case WA_RTPAGE: */
                /*             wi->left += wi->doc_width - wi->work.g_w / wi->x_fac; */
                /*             break; */

                /*         case WA_LFLINE: */
                /*             wi->left--; */
                /*             break; */

                /*         case WA_RTLINE: */
                /*             wi->left++; */
                /*             break; */
                /*     } /\* switch *\/ */
                /*     if (wi->top > wi->doc_height - wi->work.g_h / wi->y_fac) */
                /*     { */
                /*         wi->top = wi->doc_height - wi->work.g_h / wi->y_fac; */
                /*     } */
                /*     if (wi->top < 0) wi->top = 0; */
                /*     if (wi->left > wi->doc_width - wi->work.g_w / wi->x_fac) */
                /*     { */
                /*         wi->left = wi->doc_width - wi->work.g_w / wi->x_fac; */
                /*     } */
                /*     if (wi->left < 0) wi->left = 0; */
                /*     if (wi->scroll) wi->scroll(wi); */
                /*     do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h); */
                /*     break; */

                /* case WM_HSLID: */
                /*     wi->left = (int)((float) msgbuff[4] / 1000.0 * wi->doc_width); */
                /*     if (wi->scroll) wi->scroll(wi); */
                /*     do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h); */
                /*     break; */

                /* case WM_VSLID: */
                /*     wi->top = (int)((float) msgbuff[4] / 1000.0 * wi->doc_height); */
                /*     if (wi->scroll) wi->scroll(wi); */
                /*     do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h); */
                /*     break; */

                case MN_SELECTED:
                    handle_menu(gl_menu, msgbuff[3], msgbuff[4]);
                    menu_tnormal(gl_menu, msgbuff[3], true);
                    break;

                default:
                    break;
            }
        }
        else if (event & MU_TIMER)
        {
            /*
             * we send a timer event to each window that requests regular scheduling services,
             * no matter if it's on top or not
             */
            foreach_window(timer_cb);
        }
        else if (event & MU_BUTTON)
        {
            if (butdown)
            {
                butdown = 0;
            }
            else
            {
                butdown = 1;
            }
        }
        else if (event & MU_KEYBD)
	  {
	    short title, item;
	    
	    keyboard_main(keyreturn,keystate);
	    
	    if (is_menu_key(gl_menu, keyreturn, keystate, &title, &item))
	      {
	        menu_tnormal(gl_menu, title, false);
	        handle_menu(gl_menu, title, item);
	        menu_tnormal(gl_menu, title, true);
	      }
	  }
        wind_update(false);
    } while (!quit);
    
    free_windows();
    exit(0);
}

short about_exit_handler(struct dialog_handler *dial, short exit_obj)
{
    return 0;
}

short keys_exit_handler(struct dialog_handler *dial, short exit_obj)
{
    return 0;
}

short micro_exit_handler(struct dialog_handler *dial, short exit_obj)
{
    return 0;
}

short prefs_exit_handler(struct dialog_handler *dial, short exit_obj)
{
    return 0;
}

int main(int argc, char* argv[])
{
  short i;
  gl_apid = appl_init();
  phys_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
  vdi_handle=phys_handle;
  wind_get_grect(0, WF_WORKXYWH, &gl_desk);
  graf_mouse(ARROW, 0x0L);

  for (i = 0; i < 10; work_in[i++] = 1);
  work_in[10] = 2;
  v_opnvwk(work_in, &vdi_handle, work_out);

  init_global();
  init_util();
  init_windows();
  init_resource();
  init_menu();
  init_dialogs();
  
  screen_init();
  
  butdown = 1;
  quit = false;

  about_dialog = create_dialog(1,&about_exit_handler,NULL);
  prefs_dialog = create_dialog(2,&prefs_exit_handler,NULL);
  keys_dialog = create_dialog(4,&keys_exit_handler,NULL);
  micro_dialog = create_dialog(5,&micro_exit_handler,NULL);
  
  multi();

  if (gl_menu != NULL)
    {
      menu_bar(gl_menu, 0);
      gl_menu = NULL;
    }  

  screen_done();

  free_dialogs();
  free_menu();
  free_windows();
  free_resource();
  free_util();
  free_global();
  
  return 0;
}
