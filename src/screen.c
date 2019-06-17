#include "protocol.h"
#include "screen.h"
#include "splash.h"
#include "terminal.h"
#include "util.h"
#include "io.h"
#include <gem.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <osbind.h>
#include <string.h>
#include <math.h>
#include "window.h"
#include <string.h>
#include <stdio.h>
#include "config.h"

extern ConfigInfo config;

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

extern unsigned short font_fullres[];
extern unsigned short font_hires[];
extern unsigned short font_ttmedres[];
extern unsigned short font_medres[];
extern unsigned short font_lores[];

short vdi_handle;

short xoff,yoff,vxoff;
short width, height;
unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
padRGB palette[16];
unsigned char FONT_SIZE_X;
unsigned char FONT_SIZE_Y;
unsigned short* scalex;
unsigned short* scaley;
unsigned short* font;
short background_color_index=0;
short foreground_color_index=1;
padRGB background_rgb={0,0,0};
padRGB foreground_rgb={255,255,255};
unsigned char highest_color_index=0;
unsigned char being_redrawn;
short status_x;
short status_y;
short status_height;
short font_correction_y;

extern padBool FastText; /* protocol.c */
extern unsigned short fontm23[];
extern unsigned short full_screen;
extern unsigned short window_x;
extern unsigned short window_y;
extern short appl_is_mono;

static char tmptxt[80];

struct window* screen_window;

extern short work_out[57];

struct vdi_palette_entry
{
  short r;
  short g;
  short b;
};

struct vdi_palette_entry saved_palette[16];

#define VDI_COLOR_SCALE 3.91
#define PLATOTERMWINDOW_CLASS 0x7074726d // ptrm
#define PLATO_BUFFER_SIZE 16384

/**
 * screen_clip_whole_window_if_not_redrawing(void)
 * Clip the work area if window isn't being redrawn, avoiding spill-over 
 */
void screen_clip_whole_window_if_not_redrawing(short on)
{
}

/**
 * screen_strndup(ch, count) - duplicate character data.
 */
char* screen_strndup(unsigned char* ch, unsigned char count)
{
  char* ret=calloc((char)count,sizeof(char));

  if (ret==NULL)
    return NULL;

  memcpy(ret,ch,count);
  return ret;
  
}

/**
 * Screen coordinate functions
 */
short screen_x(short x)
{
  return scalex[x]+screen_window->work.g_x;
}

short screen_y(short y)
{
  return scaley[y]+screen_window->work.g_y;
}

/**
 * Window Timer callback, used to call I/O.
 */
void screen_timer(struct window* wi)
{
  io_main();
}

/**
 * Window draw callback
 */
void screen_draw(struct window* wi, short x, short y, short w, short h)
{
  struct PLATOTermWindowData* pd=wi->priv;
  being_redrawn=true;
  CurMem=M0;
  screen_window->clear(screen_window,x,y,w,h);
  ShowPLATO((padByte *)pd->platoData,pd->platoLen);
  being_redrawn=false;
}

/**
 * Window delete callback
 */
void screen_delete(struct window* wi)
{
  struct PLATOTermWindowData* pd=wi->priv;

  if (pd)
    {
      if (pd->platoData)
	free(pd->platoData);

      free(pd);
      wi->priv=NULL;
    }
  
    // Pass to window.c to pass it to AES
    delete_window(wi);
}

/**
 * screen_save_vdi_palette() - Save the VDI palette to saved_palette
 */
void screen_save_vdi_palette(void)
{
  short color[3];
  short i;
  for (i=0;i<16;i++)
    {
      vq_color(vdi_handle,i,1,color);
      saved_palette[i].r=color[0];
      saved_palette[i].g=color[1];
      saved_palette[i].b=color[2];
    }
}

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  struct PLATOTermWindowData* pd;

  screen_save_vdi_palette();
  
  width=work_out[0];
  height=work_out[1];
  
  if (width==639 && height==479)
    {
      // TT Med Res.
      scalex=scalex_ttmedres;
      scaley=scaley_ttmedres;
      font=font_ttmedres;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=14;
      vxoff=64;
      yoff=18;
      height-=18;
      status_x=535;
      status_y=yoff-4;
      status_height=14;
    }
  else if (width==639 && height==399)
    {
      // ST High res
      scalex=scalex_hires;
      scaley=scaley_hires;
      font=font_hires;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=12;
      vxoff=64;
      yoff=18;
      height-=18;
      status_x=527;
      status_y=yoff-4;
      status_height=14;
    }
  else if (width==639 && height==199)
    {
      // ST Med res
      scalex=scalex_medres;
      scaley=scaley_medres;
      font=font_medres;
      vxoff=64;
      yoff=10;
      height-=10;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=6;
      status_x=535;
      status_y=yoff-3;
      status_height=6;
    }
  else if (width==319 && height==199)
    {
      // ST low res
      scalex=scalex_lores;
      scaley=scaley_lores;
      font=font_lores;
      FONT_SIZE_X=5;
      FONT_SIZE_Y=6;
      yoff=10;
      height-=10;
      status_x=216;
      status_y=yoff-3;
      status_height=6;
      font_correction_y=1;
    }
  else
    {
      scalex=scalex_fullres;
      scaley=scaley_fullres;
      font=font_fullres;
      FONT_SIZE_X=8;
      FONT_SIZE_Y=16;
      width=height=512;
      height+=18;
      status_x=535;
      status_y=yoff-4;
      status_height=14;
    }
  
  // Set up window
  if (FONT_SIZE_Y==16)
    screen_window=create_window(9,"PLATOTERM");
  else
    screen_window=create_window(0,"PLATOTERM");
  screen_window->class=PLATOTERMWINDOW_CLASS;
  screen_window->draw=screen_draw;
  screen_window->del=screen_delete;
  screen_window->timer=screen_timer;
  pd = malloc(sizeof(struct PLATOTermWindowData));
  pd->platoData=malloc(PLATO_BUFFER_SIZE);
  pd->platoLen=0;
  memset(pd->platoData,0,PLATO_BUFFER_SIZE);
  screen_window->priv = pd;
  
  if (FONT_SIZE_Y==16)
    open_window(screen_window,10, 32, width, height);
  else
    open_window(screen_window, xoff, yoff, width, height);

  // Copy splash data to window
  memcpy(pd->platoData,(padByte *)splash,sizeof(splash));
  pd->platoLen=sizeof(splash);

    /* do_redraw(screen_window, */
    /* 	    screen_window->work.g_x, */
    /* 	    screen_window->work.g_y, */
    /* 	    screen_window->work.g_w, */
    /* 	    screen_window->work.g_h); */
  

}

/**
 * screen_update_status(void) - Update status data
 */
void screen_update_status(void)
{
  char str[64];
  char mstr[6];
  int baud_display;
  short dummy;
  
  switch (config.baud)
    {
    case 0:
      baud_display=19200;
      break;
    case 1:
      baud_display=9600;
      break;
    case 2:
      baud_display=4800;
      break;
    case 4:
      baud_display=2400;
      break;
    case 7:
      baud_display=1200;
      break;
    case 9:
      baud_display=300;
      break;
    }

  if (TTY==true)
    strcpy(mstr,"TTY  ");
  else
    strcpy(mstr,"PLATO");
  
  sprintf(str,"%s | %5d",mstr,baud_display);

  wind_update(BEG_UPDATE);

  set_clipping(vdi_handle,
	       screen_window->work.g_x,
	       screen_window->work.g_y,
	       screen_window->work.g_w,
	       screen_window->work.g_h,
	       0);
 
  vst_height(vdi_handle,status_height,&dummy,&dummy,&dummy,&dummy);
  v_gtext(vdi_handle,status_x,status_y,str);

  set_clipping(vdi_handle,
	       screen_window->work.g_x,
	       screen_window->work.g_y,
	       screen_window->work.g_w,
	       screen_window->work.g_h,
	       1);

  wind_update(END_UPDATE);
}

/**
 * screen_wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
  Bconout(2,0x07);
}

/**
 * screen_set_pen_mode - Set the VDI pen mode
 */
void screen_set_pen_mode(void)
{
  if ((CurMode==ModeErase) || (CurMode==ModeInverse))
    {
      vsf_color(vdi_handle,background_color_index); // white
      vsl_color(vdi_handle,background_color_index);
    }
  else
    {
      vsf_color(vdi_handle,foreground_color_index); // black
      vsl_color(vdi_handle,foreground_color_index);
    }

  // Also be sure to set interior to solid.
  vsf_interior(vdi_handle,1);
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  unsigned char i;
  struct PLATOTermWindowData* pd=screen_window->priv;

  screen_clip_whole_window_if_not_redrawing(true);
  
  if (!screen_window)
    return;

  if ((screen_window->topped==false))
    return;
  
  highest_color_index=0;

  for (i=0;i<16;i++)
    {
      palette[i].red=0;
      palette[i].green=0;
      palette[i].blue=0;
    }
  
  foreground_color_index=background_color_index=0;
  
  palette[0].red=background_rgb.red;
  palette[0].green=background_rgb.green;
  palette[0].blue=background_rgb.blue;

  if ((background_rgb.red   != foreground_rgb.red) &&
      (background_rgb.green != foreground_rgb.green) &&
      (background_rgb.blue  != foreground_rgb.blue))
    {
      palette[1].red=foreground_rgb.red;
      palette[1].green=foreground_rgb.green;
      palette[1].blue=foreground_rgb.blue;
      highest_color_index++;
      foreground_color_index=1;
    }

  // Finally, a fall back, if somehow, color 0 and color 1 are black, fix it.
  if ((palette[1].red==0) &&
      (palette[1].green==0) &&
      (palette[1].blue==0) &&
      (palette[0].red==0) &&
      (palette[0].green==0) &&
      (palette[0].blue==0))
    {
      palette[0].red=palette[0].green=palette[0].blue=0;
      palette[1].red=palette[1].green=palette[1].blue=255;
      foreground_color_index=1;
      background_color_index=0;
      foreground_rgb.red=foreground_rgb.green=foreground_rgb.blue=255;
      background_rgb.red=foreground_rgb.green=foreground_rgb.blue=0;
    }
  
  screen_update_colors();

  screen_set_pen_mode();
  
  screen_window->clear(screen_window,
		       screen_window->work.g_x,
		       screen_window->work.g_y,
		       screen_window->work.g_w,
		       screen_window->work.g_h);
  
  if (being_redrawn==0)
    /* // Reset the buffer. */
    pd->platoLen=0;

    screen_clip_whole_window_if_not_redrawing(false);
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  short pxyarray[4];
  
  screen_clip_whole_window_if_not_redrawing(true);

  pxyarray[0]=screen_x(min(Coord1->x,Coord2->x));
  pxyarray[1]=screen_y(min(Coord1->y,Coord2->y));
  pxyarray[2]=screen_x(max(Coord2->x,Coord1->x));
  pxyarray[3]=screen_y(max(Coord2->y,Coord1->y));
  
  /* screen_set_pen_mode(); */
  vsf_color(vdi_handle,0);
  vsf_interior(vdi_handle,1);
  vswr_mode(vdi_handle,1);
  v_bar(vdi_handle,pxyarray);

  screen_clip_whole_window_if_not_redrawing(false);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  short pxyarray[4];

  screen_clip_whole_window_if_not_redrawing(true);

  if ((being_redrawn==false) && (screen_window->topped==false))
    return;
  
  screen_set_pen_mode();

  pxyarray[0]=screen_x(Coord->x);
  pxyarray[1]=screen_y(Coord->y);
  pxyarray[2]=screen_x(Coord->x);
  pxyarray[3]=screen_y(Coord->y);
  
  v_pline(vdi_handle,2,pxyarray);

  screen_clip_whole_window_if_not_redrawing(false);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  short pxyarray[4];

  screen_clip_whole_window_if_not_redrawing(true);

  if ((being_redrawn==false) && (screen_window->topped==false))
    return;

  screen_set_pen_mode();
  
  pxyarray[0]=screen_x(Coord1->x);
  pxyarray[1]=screen_y(Coord1->y);
  pxyarray[2]=screen_x(Coord2->x);
  pxyarray[3]=screen_y(Coord2->y);

  v_pline(vdi_handle,2,pxyarray);

  screen_clip_whole_window_if_not_redrawing(false);
}

/**
 * screen_char_bold_shift() - enlarge character for bold mode
 */
void screen_char_bold_shift(unsigned short* bold_char, unsigned short* ch)
{
  unsigned short a,i,j,k=0;
  const unsigned short ANDTAB[8]={0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};
  const unsigned short DBLORTAB[8]={0x0003,0x000C,0x0030,0x00C0,0x0300,0x0C00,0x3000,0xC000};
  for (i=0;i<FONT_SIZE_Y;++i)
    {
      a=*ch++;
      bold_char[k]=bold_char[k+1]=0;
      for (j=0;j<8;++j)
	{
	  if (a&ANDTAB[j])
	    {
	      bold_char[k]|=DBLORTAB[j];
	      bold_char[k+1]|=DBLORTAB[j];
	    }
	}
      k+=2; // Should be (FONT_SIZE*2)-1 on end.
    }
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  char* chptr;
  unsigned char a;
  unsigned short* curfont;
  unsigned char i;
  short offset;
  MFDB srcMFDB, destMFDB;
  short pxyarray[8];
  short colors[2]={1,0}; // Output colors
  short current_mode=1;  // Default to Rewrite
  short bold_char[32];   // Bold character buffer.
  destMFDB.fd_addr=0; // We blit to the screen.

  screen_clip_whole_window_if_not_redrawing(true);

  if ((being_redrawn==false) && (screen_window->topped==false))
    return;
  
    // Create copy of character buffer, if queuing up.
  switch(CurMem)
    {
    case M0:
      curfont=font;
      offset=-32;
      break;
    case M1:
      curfont=font;
      offset=64;
      break;
    case M2:
      curfont=(unsigned short *)fontm23;
      offset=-32;
      break;
    case M3:
      curfont=(unsigned short *)fontm23;
      offset=32;      
      break;
    }

  switch(CurMode)
    {
    case ModeWrite:
      colors[0]=foreground_color_index;
      colors[1]=background_color_index;
      current_mode=2; // Transparent
      break;
    case ModeRewrite:
      colors[0]=foreground_color_index;
      colors[1]=background_color_index;
      current_mode=1; // Replace
      break;
    case ModeErase:
      colors[0]=background_color_index;
      colors[1]=foreground_color_index;
      current_mode=2; // Transparent
      break;
    case ModeInverse:
      colors[0]=background_color_index;
      colors[1]=foreground_color_index;
      current_mode=1; // Replace
      break;
    }

  srcMFDB.fd_wdwidth=1;
  srcMFDB.fd_stand=0;
  srcMFDB.fd_nplanes=1;
  if (ModeBold==padT)
    {
      srcMFDB.fd_w=(FONT_SIZE_X<<1)-1;
      srcMFDB.fd_h=(FONT_SIZE_Y<<1)-1;
      pxyarray[0]=pxyarray[1]=0;
      pxyarray[2]=(FONT_SIZE_X<<1)-1;
      pxyarray[3]=(FONT_SIZE_Y<<1)-1;
      pxyarray[4]=screen_x(Coord->x);
      pxyarray[5]=screen_y(Coord->y)-(FONT_SIZE_Y<<1);
      pxyarray[6]=screen_x(Coord->x)+(FONT_SIZE_X<<1)-1;
      pxyarray[7]=screen_y(Coord->y)+(FONT_SIZE_Y<<1)-1;      
    }
  else
    {
      srcMFDB.fd_w=FONT_SIZE_X-1;
      srcMFDB.fd_h=FONT_SIZE_Y-1;
      pxyarray[0]=pxyarray[1]=0;
      pxyarray[2]=FONT_SIZE_X-1;
      pxyarray[3]=FONT_SIZE_Y-1;
      pxyarray[4]=screen_x(Coord->x);
      pxyarray[5]=screen_y(Coord->y)-FONT_SIZE_Y+font_correction_y;
      pxyarray[6]=screen_x(Coord->x)+FONT_SIZE_X-1;
      pxyarray[7]=screen_y(Coord->y)+FONT_SIZE_Y-1-font_correction_y;
    }

  if (ModeBold==padT)
    {
      for (i=0;i<count;++i)
	{
	  a=*ch;
	  ++ch;
	  a+=offset;
	  screen_char_bold_shift(bold_char,&curfont[(a*FONT_SIZE_Y)]);
	  srcMFDB.fd_addr=&bold_char;
	  vrt_cpyfm(vdi_handle,current_mode,pxyarray,&srcMFDB,&destMFDB,colors);
	  pxyarray[4]+=FONT_SIZE_X+FONT_SIZE_X;
	  pxyarray[6]+=FONT_SIZE_X+FONT_SIZE_X+FONT_SIZE_X+FONT_SIZE_X;
	}
    }
  else
    {
      for (i=0;i<count;++i)
	{
	  a=*ch;
	  ++ch;
	  a+=offset;
	  srcMFDB.fd_addr=&curfont[(a*FONT_SIZE_Y)];
	  vrt_cpyfm(vdi_handle,current_mode,pxyarray,&srcMFDB,&destMFDB,colors);
	  pxyarray[4]+=FONT_SIZE_X;
	  pxyarray[6]+=FONT_SIZE_X+FONT_SIZE_X;
	}
    }

  screen_clip_whole_window_if_not_redrawing(false);
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  short pxyarray[4];
  if ((theChar >= 0x20) && (theChar < 0x7F)) {
    screen_char_draw(&TTYLoc, &theChar, 1);
    TTYLoc.x += CharWide;
  }
  else if ((theChar == 0x0b)) /* Vertical Tab */
    {
      TTYLoc.y += CharHigh;
    }
  else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
      TTYLoc.x -= CharWide;
      vsf_color(vdi_handle,0);
      vsf_interior(vdi_handle,1); // Solid interior
      pxyarray[0]=screen_x(TTYLoc.x);
      pxyarray[1]=screen_y(TTYLoc.y);
      pxyarray[2]=screen_x(TTYLoc.x+CharWide);
      pxyarray[3]=screen_y(TTYLoc.y+CharHigh);
      v_bar(vdi_handle,pxyarray);
      vsf_color(vdi_handle,1);
    }
  else if (theChar == 0x0A)			/* line feed */
    TTYLoc.y -= CharHigh;
  else if (theChar == 0x0D)			/* carriage return */
    TTYLoc.x = 0;
  
  if (TTYLoc.x + CharWide > 511) {	/* wrap at right side */
    TTYLoc.x = 0;
    TTYLoc.y -= CharHigh;
  }
  
  if (TTYLoc.y < 0) {
    screen_clear();
    TTYLoc.y=495;
  }

}

/**
 * screen_restore_vdi_palette(void) - Restore VDI from saved_palette
 */
void screen_restore_vdi_palette(void)
{
  short i;
  short color[3];
  for (i=0;i<16;i++)
    {
      color[0]=saved_palette[i].r;
      color[1]=saved_palette[i].g;
      color[2]=saved_palette[i].b;
      vs_color(vdi_handle,i,color);
    }
}

/**
 * screen_done()
 * Close down TGI
 */
void screen_done(void)
{
  screen_restore_vdi_palette();
}

/**
 * screen_color_matching(color) - return index of matching color, or a new index, 
 * if not found.
 */
unsigned char screen_color_matching(padRGB* theColor)
{
  unsigned char i;
  for (i=0;i<16;i++)
    {
      if (i>highest_color_index)
	{
	  palette[i].red=theColor->red;
	  palette[i].green=theColor->green;
	  palette[i].blue=theColor->blue;
	  highest_color_index++;
	  return i;
	}
      else
	{
	  if ((palette[i].red==theColor->red) && 
	      (palette[i].green==theColor->green) && 
	      (palette[i].blue==theColor->blue))
	    {
	      return i;
	    }
	}
    }
}

/**
 * screen_remap_palette(void)
 * Remap the screen palette
 */
void screen_update_colors(void)
{
  int i=0;
  for (i=0;i<16;++i)
    {
      short current_color[3]={palette[i].red*VDI_COLOR_SCALE,palette[i].green*VDI_COLOR_SCALE,palette[i].blue*VDI_COLOR_SCALE};
      vs_color(vdi_handle,i,current_color);
    }
}

/**
 * Set foreground color
 */
void screen_foreground(padRGB* theColor)
{
  if ((being_redrawn==false) && (screen_window->topped==false))
    return;

  foreground_rgb.red=theColor->red;
  foreground_rgb.green=theColor->green;
  foreground_rgb.blue=theColor->blue;
  foreground_color_index=screen_color_matching(theColor);
  screen_update_colors();
}

/**
 * Set background color
 */
void screen_background(padRGB* theColor)
{

  if ((being_redrawn==false) && (screen_window->topped==false))
    return;
  
  background_rgb.red=theColor->red;
  background_rgb.green=theColor->green;
  background_rgb.blue=theColor->blue;
  background_color_index=screen_color_matching(theColor);
  screen_update_colors();
}

/**
 * get screen color for mono displays
 */
short screen_color_mono(padRGB* theColor)
{
}

/**
 * paint
 */
void screen_paint(padPt* Coord)
{
  vsf_color(vdi_handle,foreground_color_index);
  vsf_interior(vdi_handle,1); // Solid interior
  v_contourfill(vdi_handle,screen_x(Coord->x),screen_y(Coord->y),background_color_index);
}

