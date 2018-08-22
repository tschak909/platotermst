#include <gem.h>
#include <windom.h>
#include <stdint.h>
#include <stdlib.h>
#include "appl.h"
#include "protocol.h"
#include "screen.h"

extern unsigned char splash[];
extern short splash_size;

int16_t magic_os=FALSE;                // Are we running under MagiC?
int16_t mint_os=FALSE;                 // Are we running under MINT?
int16_t full_screen=FALSE;             // Are we running full screen? 
int16_t appl_atari_hi_res=FALSE;       // Are we in Atari Hi Res (640x400?)
int16_t appl_atari_med_res=FALSE;      // Are we in Atari Med Res (640x200?)
int16_t appl_atari_low_res=FALSE;      // Are we in Atari Low Res (640x200?)
WINDOW* win;
MFDB* terminal_bitmap;

/**
 * Initialize the application context
 */
void applinit(void)
{
  ApplInit();

  // Detect if we are under MagiC or MINT
  magic_os = vq_magx();
  mint_os = vq_mint();

  // Determine if we need to go full screen
  full_screen=appl_get_fullscreen();
    
  // Create the window.
  win = WindCreate( NAME|MOVER|CLOSER, app.x, app.y, app.w, app.h);
  if (full_screen==TRUE)
    WindOpen( win, app.x, app.y, app.x+app.w, app.y+app.h);
  else
    WindOpen( win, app.x, app.y, 512, 512);
  WindSetStr( win, WF_NAME, "PLATOTerm ST");

  appl_fullscreen();
}

/**
 * Application main loop
 */
void applmain(void)
{
  for (;;)
    EvntWindom( MU_MESAG|MU_TIMER|MU_KEYBD|MU_BUTTON);
}

/**
 * Restore from full screen
 */
void appl_restore_screen( void)
{
	form_dial( FMD_FINISH, 0, 0, 1 + app.work_out[0], 1 + app.work_out[1], 0, 0, 1 + app.work_out[0], 1 + app.work_out[1]);
	menu_bar( app.menu, 1) ;
	v_show_c( app.aeshdl, 0);
}

/**
 * Clear screen
 */
void appl_clear_screen(void)
{
  short xy[8];
  
  vswr_mode( app.aeshdl, MD_REPLACE);
  form_dial( FMD_START, 0, 0, 1 + app.work_out[0], 1 + app.work_out[1], 0, 0, 1 + app.work_out[0], 1 + app.work_out[1]);
  vsf_perimeter( app.aeshdl, 0);
  vsf_interior( app.aeshdl, FIS_SOLID);
  xy[0] = xy[1] = 0;
  xy[2] = app.work_out[0];
  xy[3] = app.work_out[1];
  vsf_color( app.aeshdl, 0); // Needs to be white.
  vr_recfl( app.aeshdl, xy);
}

/**
 * Set the PLATO display as fullscreen
 */
void appl_fullscreen(void)
{
  short	screenw = app.work_out[0] + 1;	// bottom-right X
  short	screenh = app.work_out[1] + 1;	// bottom-right Y
  MFDB 	*out, resized_out = { NULL, screenw, screenh, 0, 0, 0, 0, 0, 0}, screen = {0};
  short	posx, posy, xy[8];
  short pxy[4];
  
  appl_clear_screen();
  wind_update(BEG_UPDATE);
  ShowPLATO((padByte *)splash,1777);
  evnt_keybd();
  appl_restore_screen();
  wind_update(END_UPDATE);
  
}

/**
 * Return whether full screen is needed.
 */
short appl_get_fullscreen(void)
{
  // Set full screen if screen is small enough.
  if (app.work_out[0]==639 && app.work_out[1]==399)
    appl_atari_hi_res=TRUE;
  else if (app.work_out[0]==639 && app.work_out[1]==199)
    appl_atari_med_res=TRUE;
  else if (app.work_out[0]==319 && app.work_out[1]==199)
    appl_atari_low_res=TRUE;

  if (appl_atari_hi_res==TRUE || appl_atari_med_res==TRUE || appl_atari_low_res==TRUE)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/**
 * Initialize the off-screen bitmap for terminal.
 */
void appl_terminal_bitmap_init(void)
{
  
}
