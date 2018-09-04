#include <gem.h>
#include <windom.h>
#include <stdint.h>
#include <stdlib.h>
#include "appl.h"
#include "protocol.h"
#include "screen.h"
#include "screen_queue.h"
#include "terminal.h"
#include <osbind.h>

extern unsigned char splash[];
extern short splash_size;
extern DrawElement* screen_queue;

extern unsigned char FONT_SIZE_X;
extern unsigned char FONT_SIZE_Y;
extern unsigned char* font;
extern unsigned char font_fullres[];
extern unsigned char font_hires[];
extern unsigned char font_ttmedres[];
extern unsigned char font_medres[];
extern unsigned char font_lores[];

extern unsigned short* scalex;
extern unsigned short* scaley;
extern unsigned short scalex_hires[];
extern unsigned short scaley_hires[];
extern unsigned short scalex_medres[];
extern unsigned short scaley_medres[];
extern unsigned short scalex_ttmedres[];
extern unsigned short scaley_ttmedres[];
extern unsigned short scalex_lores[];
extern unsigned short scaley_lores[];
extern unsigned short scalex_fullres[];
extern unsigned short scaley_fullres[];
extern unsigned short background_color[3];
extern unsigned short foreground_color[3];

int16_t magic_os=FALSE;                // Are we running under MagiC?
int16_t mint_os=FALSE;                 // Are we running under MINT?
int16_t full_screen=FALSE;             // Are we running full screen? 
int16_t appl_atari_hi_res=FALSE;       // Are we in Atari Hi Res (640x400?)
int16_t appl_atari_med_res=FALSE;      // Are we in Atari Med Res (640x200?)
int16_t appl_atari_tt_med_res=FALSE;   // Are we in Atari TT Med Res (640x480?)
int16_t appl_atari_low_res=FALSE;      // Are we in Atari Low Res (640x200?)
int16_t appl_is_mono=FALSE;            // Are we in mono? 
WINDOW* win;
int16_t window_x, window_y;            // Window coordinates
int16_t appl_init_successful=FALSE;    // Application successfully initialized.

static void appl_redraw(WINDOW* win,short wbuff[8])
{
  short xw, yw, ww, hw; // Window dimensions
  short xy[8];

  WindGet(win,WF_WORKXYWH,&xw,&yw,&ww,&hw);
  window_x=xw;
  window_y=yw;

  wind_update(BEG_UPDATE);
  appl_clear_screen();
  screen_redraw();
  wind_update(END_UPDATE);
}

/**
 * Usefull fonction to get object tree from resource.
 */

OBJECT* appl_get_tree( int index)
{
  OBJECT *tree;
  rsrc_gaddr( 0, index, &tree);
  return tree;
}

/**
 * Called for keyboard I/O
 */
static void appl_kybd(WINDOW *win, short buff[8])
{
  keyboard_main(evnt.keybd,evnt.mkstate);
}

/**
 * Called every timer cycle to update I/O
 */
static void appl_timer(WINDOW *win, short buff[8])
{
  io_main();
}

/**
 * Initialize the application context
 */
void applinit(void)
{
  short xw,yw,ww,hw;
  short cx,cy,cw,ch;
  ApplInit();
  io_init();
  if (RsrcLoad("plato.rsc") == 0)
    {
      FormAlert(1, FA_ERROR "[Can not locate PLATO.RSC][Exit]");
      ApplExit();
      appl_init_successful=false;
      return;
    }

  // Install menubar
  MenuBar(appl_get_tree(MAINMENU),1);

  /* Register application name in menu if possible */
  if( _AESnumapps == -1)
    menu_register( _AESapid, "  PLATOTerm ");

  
  // Determine if we need to go full screen
  full_screen=appl_get_fullscreen();
    
  // Create the window.
  if (full_screen==TRUE)
    win=WindCreate(0,app.x,app.y,app.w,app.h);
  else
    win = WindCreate( NAME|MOVER|CLOSER, app.x, app.y, app.w, app.h);

  wind_calc(WC_BORDER,NAME|MOVER|CLOSER,app.x,app.y,512,512,&cx,&cy,&cw,&ch);
  
  if (full_screen==TRUE)
    WindOpen( win, app.x, app.y, app.x+app.w, app.y+app.h);
  else
    WindOpen( win, cx, cy, cw, ch);

  WindSetStr( win, WF_NAME, "PLATOTerm ST");
  
  WindGet(win,WF_WORKXYWH,&xw,&yw,&ww,&hw);
  window_x=xw;
  window_y=yw;

  if (appl_is_mono)
    {
      background_color[0]=0;
      background_color[1]=0;
      background_color[2]=0;
      foreground_color[0]=1000;
      foreground_color[1]=1000;
      foreground_color[2]=1000;
    }
  
  evnt.timer=0;

  terminal_init();
  terminal_initial_position();
  
  EvntAttach(win,WM_REDRAW,appl_redraw);  
  EvntAttach(NULL, AP_TERM, appl_term);
  EvntAttach(win,WM_XTIMER,appl_timer);
  EvntAttach(win,WM_XKEYBD,appl_kybd);

  ObjcAttachMenuFunc(NULL, MENU_ABOUT, appl_about, NULL);
  ObjcAttachMenuFunc(NULL,MENU_QUIT,appl_quit_form,NULL);
  
  appl_init_successful=true;
  
}

static void __CDECL appl_quit_yes( WINDOW *win, int obj, int mode, void *data) {
	ObjcChange( OC_FORM, win, obj, 0, FALSE);
	ApplWrite( _AESapid, WM_CLOSED, win->handle, 0, 0, 0, 0);
	ApplWrite( _AESapid, AP_TERM, 0, 0, 0, 0, 0);
}

static void __CDECL appl_quit_no( WINDOW *win, int obj, int mode, void *data) {
	ObjcChange( OC_FORM, win, obj, 0, FALSE);
	ApplWrite( _AESapid, WM_CLOSED, win->handle, 0, 0, 0, 0);
}

/**
 * show quit form
 */
static void appl_quit_form(WINDOW *win, int index, int mode, void *data)
{
  OBJECT *quitform=appl_get_tree(FORM_QUIT);
  win=FormWindBegin(quitform, "Quit PLATOTerm");
  ObjcAttachFormFunc(win,BUTTON_QUIT_YES,appl_quit_yes,NULL);
  ObjcAttachFormFunc(win,BUTTON_QUIT_NO,appl_quit_no,NULL);
  FormWindDo(MU_MESAG);
  FormWindEnd();
}

/**
 * close app about menu
 */
static void appl_about_close(WINDOW *win, int index, int mode, void *data)
{
  ObjcChange(mode, win, index, 0, TRUE);
  ApplWrite(_AESapid, WM_DESTROY, win->handle, 0,0,0,0);
}

/**
 * show app about menu
 */
static void appl_about(WINDOW *null, int index, int title, void *data)
{
  OBJECT *aboutbox = appl_get_tree(FORM_ABOUT);
  FormWindBegin(aboutbox, "About PLATOTerm ST");
  FormWindDo(MU_MESAG);
  FormWindEnd();
  /* ObjcChange(OC_FORM,win,index,0,TRUE); */
}

/**
 * Application main loop
 */
void applmain(void)
{
  appl_clear_screen();
  ShowPLATO((padByte *)splash,splash_size);
  
  for (;;)
    EvntWindom( MU_MESAG|MU_TIMER|MU_KEYBD|MU_BUTTON);

}

/**
 * Restore from full screen
 */
void appl_restore_screen( void)
{
  form_dial( FMD_FINISH, 0, 0, 1 + app.work_out[0], 1 + app.work_out[1], 0, 0, 1 + app.work_out[0], 1 + app.work_out[1]);
  v_show_c( app.aeshdl, 0);
}

/**
 * Clear screen
 */
void appl_clear_screen(void)
{
  short xy[8];
  short xw,yw,ww,hw;

  if (full_screen==true)
    {
      vswr_mode( app.aeshdl, MD_REPLACE);
      form_dial( FMD_START, 0, 0, 1 + app.work_out[0], 1 + app.work_out[1], 0, 0, 1 + app.work_out[0], 1 + app.work_out[1]);
      vsf_perimeter( app.aeshdl, 0);
      vsf_color(app.aeshdl, 0);
      vsf_interior( app.aeshdl, FIS_SOLID);
      xy[0] = xy[1] = 0;
      xy[2] = app.work_out[0];
      xy[3] = app.work_out[1];
      vr_recfl(app.aeshdl,xy);
      menu_bar( app.menu, 0); // Disable menu bar in fullscreen.	
    }
  else // Windowed.
    {
      WindClear(win);
    }
}

/**
 * Set the PLATO display as fullscreen
 */
void appl_fullscreen(void)
{
  appl_clear_screen();
}

/**
 * Return whether full screen is needed.
 */
short appl_get_fullscreen(void)
{
  // Set full screen if screen is small enough.
  if (app.work_out[0]==639 && app.work_out[1]==479)
    {
      // 640x480 TT med res.
      appl_atari_tt_med_res=TRUE;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=15;
      scalex=scalex_ttmedres;
      scaley=scaley_ttmedres;
      font=font_ttmedres;
    }
  else if (app.work_out[0]==639 && app.work_out[1]==399)
    {
      // 640x400
      appl_atari_hi_res=TRUE;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=12;
      scalex=scalex_hires;
      scaley=scaley_hires;
      font=font_hires;
      appl_is_mono=TRUE;
    }
  else if (app.work_out[0]==639 && app.work_out[1]==199)
    {
      // 640x200 
      appl_atari_med_res=TRUE;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=6;
      scalex=scalex_medres;
      scaley=scaley_medres;
      font=font_medres;
    }
  else if (app.work_out[0]==319 && app.work_out[1]==199)
    {
      // 320x200
      appl_atari_low_res=TRUE;
      FONT_SIZE_X=5;
      FONT_SIZE_Y=6;
      scalex=scalex_lores;
      scaley=scaley_lores;
      font=font_lores;
    }
  else
    {
      // Windowed.
      FONT_SIZE_X=8;
      FONT_SIZE_Y=16;
      scalex=scalex_fullres;
      scaley=scaley_fullres;
      font=font_fullres;
      if (app.color<3) // mono display
	appl_is_mono=TRUE;
    }

  if (appl_atari_hi_res==TRUE || appl_atari_tt_med_res==TRUE || appl_atari_med_res==TRUE || appl_atari_low_res==TRUE)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

/*
 *	Close resources and cleanly quit application.
 */
static void __CDECL appl_term( WINDOW *win, short buff[8]) {
	while( wglb.first) {
		ApplWrite( _AESapid, WM_DESTROY, wglb.first->handle,0,0,0,0); 
		EvntWindom( MU_MESAG);
	}
	RsrcXtype( 0, NULL, 0);
	RsrcFree();
	ApplExit();
	exit(0);
}

/**
 * Finish application
 */
void appldone(void)
{
  ApplExit();
}
