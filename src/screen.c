
#include "protocol.h"
#include "math.h"
#include "appl.h"
#include "screen.h"
#include <windom.h>
#include <gem.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <osbind.h>
#include <bits/string2.h>
#include <math.h>

unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
DrawElement* screen_queue=NULL;
padRGB palette[16];
unsigned char FONT_SIZE_X;
unsigned char FONT_SIZE_Y;
unsigned short* scalex;
unsigned short* scaley;
unsigned char* font[];
short background_color_index=0;
short foreground_color_index=1;
padRGB background_rgb={0,0,0};
padRGB foreground_rgb={255,255,255};
unsigned char highestColorIndex=0;

extern padBool FastText; /* protocol.c */

extern unsigned char fontm23[];
extern unsigned short full_screen;
extern unsigned short window_x;
extern unsigned short window_y;
extern short appl_is_mono;

static char tmptxt[80];

#define VDI_COLOR_SCALE 3.91 

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
 * screen_x() - Get screen X coordinates
 */
short screen_x(short x)
{
  if (full_screen==true)
    return scalex[x];
  else
    return scalex[x]+window_x;
}

/**
 * screen_y() - Get screen Y coordinates
 */
short screen_y(short y)
{
  if (full_screen==true)
    return scaley[y];
  else
    return scaley[y]+window_y;
}


/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  screen_queue=screen_queue_create(0,0,0,0,0,NULL,0,0,0,0,0,0,NULL);
}

/**
 * screen_load_driver()
 * Load the TGI driver
 */
void screen_load_driver(void)
{
}

/**
 * screen_init_hook()
 * Called after tgi_init to set any special features, e.g. nmi trampolines.
 */
void screen_init_hook(void)
{
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
 * screen_remap_palette(void)
 * Remap the screen palette
 */
void screen_remap_palette(void)
{
  int i=0;
  for (i=0;i<16;++i)
    {
      short current_color[3]={palette[i].red*VDI_COLOR_SCALE,palette[i].green*VDI_COLOR_SCALE,palette[i].blue*VDI_COLOR_SCALE};
      vs_color(app.aeshdl,i,current_color);
    }
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  appl_clear_screen();
  memset(palette,-1,sizeof(palette));
  highestColorIndex=0;
  palette[0]=background_rgb;
  highestColorIndex++;
  if ((background_rgb.red   == foreground_rgb.red) &&
      (background_rgb.green == foreground_rgb.green) &&
      (background_rgb.blue  == foreground_rgb.blue))
    {
      palette[1]=foreground_rgb;
      highestColorIndex++;
    }
  screen_remap_palette();
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2, bool queue)
{
  short pxyarray[4];
  pxyarray[0]=screen_x(Coord1->x);
  pxyarray[1]=screen_y(Coord1->y);
  pxyarray[2]=screen_x(Coord2->x);
  pxyarray[3]=screen_y(Coord2->y);

  // initial naive implementation, draw a bunch of horizontal lines the size of bounding box.

  if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      vsf_color(app.aeshdl,background_color_index); // white
    }
  else
    {
      vsf_color(app.aeshdl,foreground_color_index); // black
    }
  
  v_bar(app.aeshdl,pxyarray);
  
  if (queue==true)
    screen_queue_append(screen_queue,SCREEN_QUEUE_BLOCK_ERASE,Coord1->x,Coord1->y,Coord2->x,Coord2->y,NULL,0,background_color_index,foreground_color_index,0,0,0);
  
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord, bool queue)
{
  short pxyarray[4];

    switch(CurMode)
    {
    case ModeWrite:
      vswr_mode(app.aeshdl,1);
      break;
    case ModeErase:
      vswr_mode(app.aeshdl,3);
      break;
    }
  
  pxyarray[0]=screen_x(Coord->x);
  pxyarray[1]=screen_y(Coord->y);
  pxyarray[2]=screen_x(Coord->x);
  pxyarray[3]=screen_y(Coord->y);

  v_pline(app.aeshdl,2,pxyarray);

  if (queue==true)
    screen_queue_append(screen_queue,SCREEN_QUEUE_DOT,Coord->x,Coord->y,0,0,NULL,0,background_color_index,foreground_color_index,0,0,0);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2, bool queue)
{
  short pxyarray[4];

  switch(CurMode)
    {
    case ModeWrite:
      vsl_color(app.aeshdl,foreground_color_index);
      break;
    case ModeErase:
      vsl_color(app.aeshdl,background_color_index);
      break;
    }

  
  pxyarray[0]=screen_x(Coord1->x);
  pxyarray[1]=screen_y(Coord1->y);
  pxyarray[2]=screen_x(Coord2->x);
  pxyarray[3]=screen_y(Coord2->y);

   v_pline(app.aeshdl,2,pxyarray);
   if (queue==true)
     {
       screen_queue_append(screen_queue,SCREEN_QUEUE_LINE,Coord1->x,Coord1->y,Coord2->x,Coord2->y,NULL,0,background_color_index,foreground_color_index,0,0,0);
     }
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
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count, bool queue)
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
  
  // Create copy of character buffer, if queuing up.
  if (queue==TRUE)
    chptr=screen_strndup(ch,count);
  
  switch(CurMem)
    {
    case M0:
      curfont=(unsigned short *)*font;
      offset=-32;
      break;
    case M1:
      curfont=(unsigned short *)*font;
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
      srcMFDB.fd_w=(FONT_SIZE_X*2)-1;
      srcMFDB.fd_h=(FONT_SIZE_Y*2)-1;
      pxyarray[0]=pxyarray[1]=0;
      pxyarray[2]=(FONT_SIZE_X*2)-1;
      pxyarray[3]=(FONT_SIZE_Y*2)-1;
      pxyarray[4]=screen_x(Coord->x);
      pxyarray[5]=screen_y(Coord->y)-(FONT_SIZE_Y*2);
      pxyarray[6]=screen_x(Coord->x)+(FONT_SIZE_X*2)-1;
      pxyarray[7]=screen_y(Coord->y)+(FONT_SIZE_Y*2)-1;      
    }
  else
    {
      srcMFDB.fd_w=FONT_SIZE_X-1;
      srcMFDB.fd_h=FONT_SIZE_Y-1;
      pxyarray[0]=pxyarray[1]=0;
      pxyarray[2]=FONT_SIZE_X-1;
      pxyarray[3]=FONT_SIZE_Y-1;
      pxyarray[4]=screen_x(Coord->x);
      pxyarray[5]=screen_y(Coord->y)-FONT_SIZE_Y;
      pxyarray[6]=screen_x(Coord->x)+FONT_SIZE_X-1;
      pxyarray[7]=screen_y(Coord->y)+FONT_SIZE_Y-1;
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
	  vrt_cpyfm(app.aeshdl,current_mode,pxyarray,&srcMFDB,&destMFDB,colors);
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
	  vrt_cpyfm(app.aeshdl,current_mode,pxyarray,&srcMFDB,&destMFDB,colors);
	  pxyarray[4]+=FONT_SIZE_X;
	  pxyarray[6]+=FONT_SIZE_X+FONT_SIZE_X;
	}
    }
  if (queue==true)
    {
      screen_queue_append(screen_queue,SCREEN_QUEUE_CHAR,Coord->x,Coord->y,0,0,chptr,count,background_color_index,foreground_color_index,0,0,0);
    }
  
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  short pxyarray[4];
  if ((theChar >= 0x20) && (theChar < 0x7F)) {
    screen_char_draw(&TTYLoc, &theChar, 1, true);
    TTYLoc.x += CharWide;
  }
  else if ((theChar == 0x0b)) /* Vertical Tab */
    {
      TTYLoc.y += CharHigh;
    }
  else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
      TTYLoc.x -= CharWide;
      vsf_color(app.aeshdl,0);
      vsf_interior(app.aeshdl,1); // Solid interior
      pxyarray[0]=screen_x(TTYLoc.x);
      pxyarray[1]=screen_y(TTYLoc.y);
      pxyarray[2]=screen_x(TTYLoc.x+CharWide);
      pxyarray[3]=screen_y(TTYLoc.y+CharHigh);
      v_bar(app.aeshdl,pxyarray);
      vsf_color(app.aeshdl,1);
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
 * screen_done()
 * Close down TGI
 */
void screen_done(void)
{
}

/**
 * Do next redraw
 */
void screen_next_redraw(DrawElement* element)
{
  padPt coord1, coord2;

  CurMode=element->CurMode;
  CurMem=element->CurMem;
  
  switch(element->mode)
    {
    case SCREEN_QUEUE_DOT:
      coord1.x = element->x1;
      coord1.y = element->y1;
      screen_dot_draw(&coord1,false);
      break;
    case SCREEN_QUEUE_LINE:
      coord1.x = element->x1;
      coord1.y = element->y1;
      coord2.x = element->x2;
      coord2.y = element->y2;
      screen_line_draw(&coord1,&coord2,false);
      break;
    case SCREEN_QUEUE_CHAR:
      coord1.x = element->x1;
      coord1.y = element->y1;
      screen_char_draw(&coord1,element->ch,element->chlen,false);
      break;
    case SCREEN_QUEUE_BLOCK_ERASE:
      coord1.x = element->x1;
      coord1.y = element->y1;
      coord2.x = element->x2;
      coord2.y = element->y2;
      screen_block_draw(&coord1,&coord2,false);
      break;
    case SCREEN_QUEUE_PAINT:
      coord1.x=element->x1;
      coord2.y=element->y1;
      vsf_color(app.aeshdl,element->foreground_color_index);
      vsl_color(app.aeshdl,element->foreground_color_index);
      screen_paint(&coord1,false);
      break;
   
    }
}

/**
 * screen_redraw()
 */
void screen_redraw(void)
{
  unsigned short current_color[3];
  unsigned short i=0;
  DrawElement* redraw_cursor = screen_queue;

  screen_remap_palette();
  
  while(redraw_cursor != NULL)
    {
      screen_next_redraw(redraw_cursor);
      redraw_cursor=redraw_cursor->next;
    }

}

/**
 * Screen palette dump - remove when working
 */
void screen_palette_dump(void)
{
  int i=0;
  int x=1;
  int y=32;
  short pxyarray[4]={1,200,0,0};
  for (i=0;i<16;++i)
    {
      vsf_color(app.aeshdl,i);
      vsl_color(app.aeshdl,1);
      pxyarray[0]=x;
      pxyarray[1]=y;
      pxyarray[2]=x+32;
      pxyarray[3]=y+32;
      v_bar(app.aeshdl,pxyarray);
      x+=32;
    }
  vsf_color(app.aeshdl,foreground_color_index);
  vsl_color(app.aeshdl,foreground_color_index);
    
}

/**
 * Set foreground color
 */
void screen_foreground(padRGB* theColor)
{
  short index;
  if (appl_is_mono==1)
    {
      screen_color_mono(theColor);
      return;
    }
  // otherwise, handle via palette based color setting.
  index=screen_color(theColor);
  foreground_color_index=index;
  foreground_rgb.red=theColor->red;
  foreground_rgb.green=theColor->green;
  foreground_rgb.blue=theColor->blue;
}

/**
 * Set background color
 */
void screen_background(padRGB* theColor)
{
  short index;
  if (appl_is_mono==1)
    {
      screen_color_mono(theColor);
      return;
    }
  // otherwise, handle via palette based color setting.
  index=screen_color(theColor);
  background_color_index=index;
  background_rgb.red=theColor->red;
  background_rgb.green=theColor->green;
  background_rgb.blue=theColor->blue;
}

/**
 * screen_color_matching(theColor)
 */
short screen_color_matching(padRGB* theColor)
{
  int i=0;
  for (i=0;i<16;++i)
    {
      if ((palette[i].red  ==(theColor->red)) &&
	  (palette[i].green==(theColor->green)) &&
	  (palette[i].blue ==(theColor->blue)))
        return i;
      else if (i>highestColorIndex)
        return ++highestColorIndex;
    }
  return -1;
}

/**
 * get screen color for mono displays
 */
short screen_color_mono(padRGB* theColor)
{
  if ((theColor->red == 0x00) &&
      (theColor->green == 0x00) &&
      (theColor->blue == 0x00))
    {
      return background_color_index;
    }
  return foreground_color_index;
}

/**
 * Set selected screen color (fg/bg)
 */
short screen_color(padRGB* theColor)
{
  short index=screen_color_matching(theColor);
  palette[index].red=theColor->red;
  palette[index].green=theColor->green;
  palette[index].blue=theColor->blue;
  screen_remap_palette();
  return index;
}

/**
 * paint
 */
void screen_paint(padPt* Coord, bool queue)
{
  if (appl_is_mono==1)
    {
      vsf_color(app.aeshdl,foreground_color_index);
      v_contourfill(app.aeshdl,screen_x(Coord->x),screen_y(Coord->y),-1);
      if (queue==true)
	{
	  screen_queue_append(screen_queue,SCREEN_QUEUE_PAINT,Coord->x,Coord->y,0,0,NULL,0,background_color_index,foreground_color_index,0,0,0);
	}
    }
  else
    {
      if (queue==true)
	{
	  screen_queue_append(screen_queue,SCREEN_QUEUE_PAINT,Coord->x,Coord->y,0,0,NULL,0,background_color_index,foreground_color_index,0,0,0);
	}
      vsf_color(app.aeshdl,foreground_color_index);
      v_contourfill(app.aeshdl,screen_x(Coord->x),screen_y(Coord->y),background_color_index);
    }
}
