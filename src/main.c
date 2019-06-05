#include <stdbool.h>
#include <gem.h>
#include "protocol.h"
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"
#include "util.h"
#include "window.h"

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

int main(int argc, char* argv[])
{
  gl_apid = appl_init();
  phys_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
  wind_get_grect(0, WF_WORKXYWH, &gl_desk);
  graf_mouse(ARROW, 0x0L);
  vdi_handle = open_vwork(work_out);

  init_global();
  init_util();
  init_windows();
  init_resource();
  init_menu();
  
  return 0;
}
