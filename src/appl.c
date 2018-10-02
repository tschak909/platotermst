#include <gem.h>
#include <windom.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "appl.h"
#include "protocol.h"
#include "screen.h"
#include "terminal.h"
#include "config.h"
#include <osbind.h>

extern unsigned char splash[];
extern short splash_size;

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

extern ConfigInfo config;

int16_t magic_os=FALSE;                  // Are we running under MagiC?
int16_t mint_os=FALSE;                   // Are we running under MINT?
int16_t full_screen=FALSE;               // Are we running full screen? 
int16_t appl_atari_hi_res=FALSE;         // Are we in Atari Hi Res (640x400?)
int16_t appl_atari_med_res=FALSE;        // Are we in Atari Med Res (640x200?)
int16_t appl_atari_tt_med_res=FALSE;     // Are we in Atari TT Med Res (640x480?)
int16_t appl_atari_low_res=FALSE;        // Are we in Atari Low Res (640x200?)
int16_t appl_is_mono=FALSE;              // Are we in mono? 
WINDOW* win;
int16_t window_x, window_y;              // Window coordinates
int16_t appl_init_successful=FALSE;      // Application successfully initialized.
int16_t on_top=FALSE;                    // Application on top?
int16_t appl_screen_visible=false;       // Is terminal visible?

static void appl_moved(WINDOW* win, short wbuff[8])
{
  short xw, yw, ww, hw;
  WindGet(win,WF_WORKXYWH,&xw,&yw,&ww,&hw);
  window_x=xw;
  window_y=yw;
}

static void appl_ontop(WINDOW* win, short wbuff[8])
{
  WindSet(win,WF_TOP,0,0,0,0);
  appl_clear_screen();
  on_top=TRUE;
  /* screen_remap_palette(); */
}

static void appl_offtop(WINDOW* win, short wbuff[8])
{
  WindSet(win,WF_BOTTOM,0,0,0,0);
  on_top=FALSE;
}

static void appl_redraw(WINDOW* win,short wbuff[8])
{
  appl_clear_screen();
  screen_redraw();
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

  /* Register application name in menu if possible */
  if( _AESnumapps == -1)
    menu_register( _AESapid, "  PLATOTerm ");

  
  // Determine if we need to go full screen
  full_screen=appl_get_fullscreen();
    
  // Create the window.
  if (full_screen==TRUE)
    {
      win=WindCreate(0,app.x,app.y,app.w,app.h);
      WindOpen( win, app.x, app.y, app.x+app.w, app.y+app.h);
      appl_fullscreen();
    }
  else
    {
      win = WindCreate( NAME|MOVER|CLOSER, app.x+10, app.y+10, app.x+512+10, app.y+512+10);
      WindOpen( win, app.x+10,app.y+10,app.x+512+10,app.y+512+10);
    }

  MenuBar(appl_get_tree(MAINMENU),1);
  WindSetStr( win, WF_NAME, "PLATOTerm ST");
  
  WindGet(win,WF_WORKXYWH,&xw,&yw,&ww,&hw);
  window_x=xw;
  window_y=yw;

  evnt.timer=0;

  terminal_init();
  terminal_initial_position();

  // Attach to window manager messages
  EvntAttach(win,WM_TOPPED,appl_ontop);
  EvntAttach(win,WM_UNTOPPED,appl_offtop);
  EvntAttach(win,WM_REDRAW,appl_redraw);
  EvntAttach(win,WM_MOVED,appl_moved);
  EvntAttach(NULL, AP_TERM, appl_term);
  EvntAttach(win,WM_XTIMER,appl_timer);
  EvntAttach(win,WM_XKEYBD,appl_kybd);

  ObjcAttachMenuFunc(NULL, MENU_ABOUT, appl_menu_about, NULL);
  ObjcAttachMenuFunc(NULL, MENU_BAUD_RATE, appl_menu_baud, NULL);
  ObjcAttachMenuFunc(NULL, MENU_HANG_UP, appl_menu_hang_up, NULL);
  ObjcAttachMenuFunc(NULL,MENU_QUIT,appl_menu_quit,NULL);
  ObjcAttachMenuFunc(NULL,MENU_PLATO_KEYBOARD,appl_menu_help_keys,NULL);
  ObjcAttachMenuFunc(NULL,MENU_MICRO_KEYS,appl_menu_micro_keys,NULL);
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
static void appl_menu_quit(WINDOW *win, int index, int mode, void *data)
{
  MenuTnormal(NULL,index,1);
  appl_form_quit();
}

/**
 * show hang-up alert
 */
static void appl_menu_hang_up(WINDOW *win, int index, int mode, void *data)
{
  MenuTnormal(NULL,index,1);
  appl_hang_up();
}


void appl_form_quit(void)
{
  if (FormAlert(2,"[1][Quit PLATOTerm?][Yes|No]")==1)
    {
      io_hang_up();
      ApplWrite( _AESapid, WM_CLOSED, win->handle, 0, 0, 0, 0);
      ApplWrite( _AESapid, AP_TERM, 0, 0, 0, 0, 0);
    }
}


/**
 * show app about menu
 */
static void appl_menu_about(WINDOW *win, int index, int title, void *data)
{
  MenuTnormal(NULL,index,1);
  FormAlert(1,"[2][PLATOTerm 0.5|A PLATO Terminal Emulator|by Thomas Cherryhomes|Copyright (c) 2018 IRATA.ONLINE][OK]");
}

/**
 * Show key help form
 */
void appl_menu_help_keys(WINDOW* win, int index, int mode, void *data)
{
  OBJECT* form = appl_get_tree(FORM_HELP_KEYS);
  MenuTnormal(NULL,index,1);
  FormWindBegin(form,"PLATO Keys");
  FormWindDo(MU_MESAG);
  FormWindEnd();
}

/**
 * show app about menu
 */
static void appl_menu_micro_keys(WINDOW *null, int index, int title, void *data)
{
  OBJECT* form = appl_get_tree(FORM_MICRO_KEYS);
  MenuTnormal(NULL,index,1);
  FormWindBegin(form,"MICRO Keys");
  FormWindDo(MU_MESAG);
  FormWindEnd();
}

/**
 * Show baud rate form
 */
void appl_form_baud(void)
{
  OBJECT* baud = appl_get_tree(FORM_BAUD);
  WINDOW* localwin;
  int res;
  int new_baud;
  
  localwin=FormWindBegin(baud, "Set Baud Rate");
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_300,&new_baud,9);
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_1200,&new_baud,7);
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_2400,&new_baud,4);
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_4800,&new_baud,2);
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_9600,&new_baud,1);
  ObjcAttachVar(OC_FORM,localwin,BUTTON_BAUD_19200,&new_baud,0);

  // Clear radio button widgets
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_OK, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_CANCEL, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_19200, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_9600, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_4800, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_2400, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_1200, 0, FALSE);
  ObjcChange( OC_FORM, localwin, BUTTON_BAUD_300, 0, FALSE);

  // And set the appropriate one.
  switch(config.baud)
    {
    case 0: // 19200
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_19200, 1, FALSE);
      break;
    case 1: // 9600
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_9600, 1, FALSE);
      break;
    case 2: // 4800
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_4800, 1, FALSE);
      break;
    case 4: // 2400
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_2400, 1, FALSE);
      break;
    case 7: // 1200
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_1200, 1, FALSE);
      break;
    case 9: // 300
      ObjcChange( OC_FORM, localwin, BUTTON_BAUD_300, 1, FALSE);
      break;
    }

  strcpy(ObjcString(FORM(localwin),10,NULL),config.init_str);
  strcpy(ObjcString(FORM(localwin),13,NULL),config.dial_str);
  
  res=FormWindDo(MU_MESAG);

  switch(res)
    {
    case 1:
      config.baud=new_baud;
      strcpy(config.init_str,ObjcString(FORM(localwin),10,NULL));
      strcpy(config.dial_str,ObjcString(FORM(localwin),13,NULL));
      config_save();
      /* screen_remap_palette(); */
      io_configure();
      break;
    case 2:
      /* screen_remap_palette(); */
      break;
    }
  
  FormWindEnd();
  appl_fullscreen();
}

/**
 * baud rate selected from menu.
 */
static void appl_menu_baud(WINDOW *localwin, int index, int title, void *data)
{
  MenuTnormal(NULL,index,1);
  appl_form_baud();
}

/**
 * Application main loop
 */
void applmain(void)
{ 
  for (;;)
    EvntWindom( MU_MESAG|MU_TIMER|MU_KEYBD|MU_BUTTON);
}

void appl_show_menu(void)
{
  MenuEnable();
}

void appl_hide_menu(void)
{
  MenuDisable();
}

/**
 * Restore from full screen
 */
void appl_restore_screen( void)
{
  form_dial( FMD_FINISH, 0, 0, 1 + app.work_out[0], 1 + app.work_out[1], 0, 0, 1 + app.work_out[0], 1 + app.work_out[1]);
  v_show_c( app.aeshdl, 0);
  appl_screen_visible=false;
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
    }
  else // Windowed.
    {
      wind_update(BEG_UPDATE);
      WindClear(win);
      wind_update(END_UPDATE);
    }
}

/**
 * Set the PLATO display as fullscreen
 */
void appl_fullscreen(void)
{
  appl_clear_screen();
  v_show_c( app.aeshdl, 1);
  appl_screen_visible=true;
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
static void __CDECL appl_term( WINDOW *localwin, short buff[8]) {
	while( wglb.first) {
		ApplWrite( _AESapid, WM_DESTROY, wglb.first->handle,0,0,0,0); 
		EvntWindom( MU_MESAG);
	}
	MenuBar(appl_get_tree(MAINMENU),0);
	RsrcXtype( 0, NULL, 0);
	RsrcFree();
	ApplExit();
	exit(0);
}

/**
 * Alert for hang-up and optionally, hang-up.
 */
void appl_hang_up(void)
{
  if (FormAlert(2,"[1][Hang up?][Yes|No]")==1)
    {
      io_hang_up();
    }
  
}

/**
 * Show ready prompt
 */
void appl_show_ready(void)
{
  vst_color(app.aeshdl,1);
  if (appl_atari_low_res==true)
    {
      v_gtext(app.aeshdl,0,184,"PLATOTerm Ready. F1 for Settings.");
      v_gtext(app.aeshdl,0,192,"HELP for keys. F10 to exit.");
    }
  else
    {
      v_gtext(app.aeshdl,64,app.work_out[1]-12,"PLATOTerm Ready. F1 for Settings. HELP for keys. F10 to exit.");
    }
}

/**
 * Finish application
 */
void appldone(void)
{
  ApplExit();
}
