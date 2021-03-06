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
#include "touch.h"
#include "config.h"

ConfigInfo config;

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
static short mb;            /* mouse button */
static short ret;           /* dummy return variable */

struct dialog_handler* about_dialog;
struct dialog_handler* prefs_dialog;
struct dialog_handler* keys_dialog;
struct dialog_handler* micro_dialog;
struct dialog_handler* dial_dialog;
struct dialog_handler* change_dialog;

bool quit;
unsigned char dial_dialog_active;

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
        event = evnt_multi(MU_MESAG | MU_BUTTON | MU_KEYBD | MU_TIMER,
                        1, 1, butdown,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        msgbuff, msec, &mx, &my, &mb, &keystate, &keyreturn, &ret);

        wind_update(true);

        if (event & MU_BUTTON)
        {
	  if (butdown)
            {
	      butdown = 0;
            }
	  else
            {
	      butdown = 1;
            }
	  touch_main(mx,my);
        }
	else if (event & MU_KEYBD)
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
		    screen_restore_vdi_palette();
                    break;

                case WM_SIZED:
                case WM_MOVED:
                    if (wi->size) wi->size(wi, msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                    break;

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
}

void init_change_dialog(unsigned char selected_entry, char* name, char* dialout)
{
  memset(change_dialog->dialog_object[4].ob_spec.tedinfo->te_ptext,0,24);
  memset(change_dialog->dialog_object[7].ob_spec.tedinfo->te_ptext,0,24);

  change_dialog->dialog_object[4].ob_spec.tedinfo->te_txtlen = 24;
  change_dialog->dialog_object[7].ob_spec.tedinfo->te_txtlen = 24;

  if (selected_entry==5)
    {
      memcpy(change_dialog->dialog_object[4].ob_spec.tedinfo->te_ptext,
	     config.entry1_name,
	     strlen(config.entry1_name));

      memcpy(change_dialog->dialog_object[7].ob_spec.tedinfo->te_ptext,
	     config.entry1_dial,
	     strlen(config.entry1_dial));
    }
  else if (selected_entry==6)
    {
      memcpy(change_dialog->dialog_object[4].ob_spec.tedinfo->te_ptext,
	     config.entry2_name,
	     strlen(config.entry2_name));

      memcpy(change_dialog->dialog_object[7].ob_spec.tedinfo->te_ptext,
	     config.entry2_dial,
	     strlen(config.entry2_dial));

    }
  else if (selected_entry==7)
    {
      memcpy(change_dialog->dialog_object[4].ob_spec.tedinfo->te_ptext,
	     config.entry3_name,
	     strlen(config.entry3_name));

      memcpy(change_dialog->dialog_object[7].ob_spec.tedinfo->te_ptext,
	     config.entry3_dial,
	     strlen(config.entry3_dial));
    }
  else if (selected_entry==8)
    {
      memcpy(change_dialog->dialog_object[4].ob_spec.tedinfo->te_ptext,
	     config.entry4_name,
	     strlen(config.entry4_name));

      memcpy(change_dialog->dialog_object[7].ob_spec.tedinfo->te_ptext,
	     config.entry4_dial,
	     strlen(config.entry4_dial));
    }
}

void init_dial_dialog(void)
{
  dial_dialog->dialog_object[5].ob_spec = (OBSPEC)config.entry1_name;
  dial_dialog->dialog_object[6].ob_spec = (OBSPEC)config.entry2_name;
  dial_dialog->dialog_object[7].ob_spec = (OBSPEC)config.entry3_name;
  dial_dialog->dialog_object[8].ob_spec = (OBSPEC)config.entry4_name;
  
  // Select the initial entry to dial.
  dial_dialog->dialog_object[config.entry_selected].ob_state |= OS_SELECTED;
  
}

void init_prefs_dialog(void)
{
  int sbr;
  int ssp;
  
  // Set radio button for baud.
  switch (config.baud)
    {
    case 0: // 19200
      sbr=8;
      break;
    case 1: // 9600
      sbr=7;
      break;
    case 2: // 4800
      sbr=6;
      break;
    case 4: // 2400
      sbr=5;
      break;
    case 7: // 1200
      sbr=4;
      break;
    case 9: // 300
      sbr=3;
      break;
    }

  switch (config.serial_port)
    {
    case 6:
      ssp=14;
      break;
    case 7:
      ssp=15;
      break;
    case 8:
      ssp=16;
      break;
    case 9:
      ssp=17;
      break;
    }
  
  prefs_dialog->dialog_object[sbr].ob_state |= OS_SELECTED;
  prefs_dialog->dialog_object[ssp].ob_state |= OS_SELECTED;
  
  memset(prefs_dialog->dialog_object[10].ob_spec.tedinfo->te_ptext,0,
	 prefs_dialog->dialog_object[10].ob_spec.tedinfo->te_txtlen);
  
  memcpy(prefs_dialog->dialog_object[10].ob_spec.tedinfo->te_ptext,
	 config.init_str,
	 strlen(config.init_str));
  
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

void change_fix_empty_entries(struct dialog_handler *dial)
{
  if (strlen(dial->dialog_object[4].ob_spec.tedinfo->te_ptext)==0)
    strcpy(dial->dialog_object[4].ob_spec.tedinfo->te_ptext,"EMPTY");
  if (strlen(dial->dialog_object[7].ob_spec.tedinfo->te_ptext)==0)
    strcpy(dial->dialog_object[7].ob_spec.tedinfo->te_ptext,"ATDTEXAMPLE.COM:8005");
}

short change_exit_handler(struct dialog_handler *dial, short exit_obj)
{
  if (exit_obj==8) // OK
    {
      // update the config entry.
      change_fix_empty_entries(dial);
      if (dial_dialog->dialog_object[5].ob_state & OS_SELECTED)
	{
	  strcpy(config.entry1_name,
		 dial->dialog_object[4].ob_spec.tedinfo->te_ptext);
	  strcpy(config.entry1_dial,
		 dial->dialog_object[7].ob_spec.tedinfo->te_ptext);
	}
      else if (dial_dialog->dialog_object[6].ob_state & OS_SELECTED)
	{
	  strcpy(config.entry2_name,
		 dial->dialog_object[4].ob_spec.tedinfo->te_ptext);
	  strcpy(config.entry2_dial,
		 dial->dialog_object[7].ob_spec.tedinfo->te_ptext);
	}
      else if (dial_dialog->dialog_object[7].ob_state & OS_SELECTED)
	{
	  strcpy(config.entry3_name,
		 dial->dialog_object[4].ob_spec.tedinfo->te_ptext);
	  strcpy(config.entry3_dial,
		 dial->dialog_object[7].ob_spec.tedinfo->te_ptext);
	}
      else if (dial_dialog->dialog_object[8].ob_state & OS_SELECTED)
	{
	  strcpy(config.entry4_name,
		 dial->dialog_object[4].ob_spec.tedinfo->te_ptext);
	  strcpy(config.entry4_dial,
		 dial->dialog_object[7].ob_spec.tedinfo->te_ptext);
	}
      config_save();
    }
    
  return 0;
}

short dial_exit_handler(struct dialog_handler *dial, short exit_obj)
{
  char name[24];
  char dialout[24];
  unsigned char entry_selected;
  unsigned char i;
  
  // First, figure out which dial radio is selected
  if (dial->dialog_object[5].ob_state & OS_SELECTED)
    {
      strcpy(name,config.entry1_name);
      strcpy(dialout,config.entry1_dial);
      entry_selected=5;
    }
  else if (dial->dialog_object[6].ob_state & OS_SELECTED)
    {
      strcpy(name,config.entry2_name);
      strcpy(dialout,config.entry2_dial);
      entry_selected=6;
    }
  else if (dial->dialog_object[7].ob_state & OS_SELECTED)
    {
      strcpy(name,config.entry3_name);
      strcpy(dialout,config.entry3_dial);
      entry_selected=7;
    }
  else if (dial->dialog_object[8].ob_state & OS_SELECTED)
    {
      strcpy(name,config.entry4_name);
      strcpy(dialout,config.entry4_dial);
      entry_selected=8;
    }
  
  if (exit_obj==1) // Dial
    {
      config.entry_selected=entry_selected;
      config_save();
      for (i=0;i<strlen(dialout);i++)
	io_send_byte(dialout[i]);
      io_send_byte(0x0D);
      dial_dialog_active=0;
    }
  else if (exit_obj==2) // Change
    {
      init_change_dialog(entry_selected,name,dialout);
      change_dialog->dialog_do(change_dialog);
    }
  else if (exit_obj==3) // Exit
    {
      dial_dialog_active=0;
    }
  
  // Clear the radio buttons, they will be re-instantiated if needed.
  dial->dialog_object[5].ob_state &= ~OS_SELECTED;
  dial->dialog_object[6].ob_state &= ~OS_SELECTED;
  dial->dialog_object[7].ob_state &= ~OS_SELECTED;
  dial->dialog_object[8].ob_state &= ~OS_SELECTED;
  
  return 0;
}

short prefs_exit_handler(struct dialog_handler *dial, short exit_obj)
{
  if (exit_obj==1)
    {
      // Otherwise, user selected OK; apply new values.
      if (dial->dialog_object[3].ob_state & OS_SELECTED)
	config.baud=9; // 300
      else if (dial->dialog_object[4].ob_state & OS_SELECTED)
	config.baud=7; // 1200
      else if (dial->dialog_object[5].ob_state & OS_SELECTED)
	config.baud=4; // 2400
      else if (dial->dialog_object[6].ob_state & OS_SELECTED)
	config.baud=2; // 4800
      else if (dial->dialog_object[7].ob_state & OS_SELECTED)
	config.baud=1; // 9600 
      else if (dial->dialog_object[8].ob_state & OS_SELECTED)
	config.baud=0; // 19200
      else
	config.baud=0; // might as well.

      // Serial port radio dialog.
      if (dial->dialog_object[14].ob_state & OS_SELECTED)
	config.serial_port=6;
      else if (dial->dialog_object[15].ob_state & OS_SELECTED)
	config.serial_port=7;
      else if (dial->dialog_object[16].ob_state & OS_SELECTED)
	config.serial_port=8;
      else if (dial->dialog_object[17].ob_state & OS_SELECTED)
	config.serial_port=9;
      
      // And copy the text fields back into config
      strcpy(config.init_str,
	     dial->dialog_object[10].ob_spec.tedinfo->te_ptext);
      
      config_save();
      screen_update_status();
      io_done();
      io_init();
    }

  // Clear the radio buttons, they will be re-instated
  // If the dialog is re-opened.
  dial->dialog_object[3].ob_state &= ~OS_SELECTED;
  dial->dialog_object[4].ob_state &= ~OS_SELECTED;
  dial->dialog_object[5].ob_state &= ~OS_SELECTED;
  dial->dialog_object[6].ob_state &= ~OS_SELECTED;
  dial->dialog_object[7].ob_state &= ~OS_SELECTED;
  dial->dialog_object[8].ob_state &= ~OS_SELECTED;
  dial->dialog_object[14].ob_state &= ~OS_SELECTED;
  dial->dialog_object[15].ob_state &= ~OS_SELECTED;
  dial->dialog_object[16].ob_state &= ~OS_SELECTED;
  dial->dialog_object[17].ob_state &= ~OS_SELECTED;

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

  config_init();
  io_init();
  
  screen_init();

  screen_update_status();
  
  butdown = 1;
  quit = false;

  about_dialog = create_dialog(1,&about_exit_handler,NULL);
  prefs_dialog = create_dialog(2,&prefs_exit_handler,NULL);
  keys_dialog = create_dialog(4,&keys_exit_handler,NULL);
  micro_dialog = create_dialog(5,&micro_exit_handler,NULL);
  dial_dialog = create_dialog(6,&dial_exit_handler,NULL);
  change_dialog = create_dialog(7,&change_exit_handler,NULL);
  
  init_prefs_dialog();
  
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
