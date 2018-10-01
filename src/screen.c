#include "protocol.h"
#include "math.h"
#include "appl.h"
#include "screen.h"
#include "terminal.h"
#include "screen_queue.h"
#include <windom.h>
#include <gem.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <osbind.h>
#include <bits/string2.h>
#include <math.h>

extern WINDOW* win;
unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
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

ScreenOpNode* screen_queue;

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
  short xw,yw,ww,hw;
  WindGet(win,WF_WORKXYWH,&xw,&yw,&ww,&hw);
  window_x=xw;
  window_y=yw;

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
  screen_queue_init(&screen_queue);
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
  wind_update(BEG_UPDATE);
  for (i=0;i<16;++i)
    {
      short current_color[3]={palette[i].red*VDI_COLOR_SCALE,palette[i].green*VDI_COLOR_SCALE,palette[i].blue*VDI_COLOR_SCALE};
      vs_color(app.aeshdl,i,current_color);
    }
  wind_update(END_UPDATE);
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  screen_queue_free_list(screen_queue);
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  ScreenOp op;
  op.type=SCREEN_OP_BLOCK_DRAW;
  op.Coord1.x = Coord1->x;
  op.Coord1.y = Coord1->y;
  op.Coord2.x = Coord2->x;
  op.Coord2.y = Coord2->y;
  op.foreground = foreground_rgb;
  op.background = background_rgb;
  op.CurMode = CurMode;
  screen_queue_add(screen_queue,op);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  ScreenOp op;
  op.type = SCREEN_OP_DOT;
  op.Coord1.x = Coord->x;
  op.Coord1.y = Coord->y;
  op.foreground = foreground_rgb;
  op.background = background_rgb;
  op.CurMode = CurMode;
  screen_queue_add(screen_queue,op);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  ScreenOp op;
  op.type = SCREEN_OP_LINE;
  op.Coord1.x = Coord1->x;
  op.Coord1.y = Coord1->y;
  op.Coord2.x = Coord2->x;
  op.Coord2.y = Coord2->y;
  op.foreground = foreground_rgb;
  op.background = background_rgb;
  op.CurMode = CurMode;
  screen_queue_add(screen_queue,op);
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
  ScreenOp op;
  op.type = SCREEN_OP_ALPHA;
  op.Coord1.x = Coord->x;
  op.Coord1.y = Coord->y;
  op.foreground = foreground_rgb;
  op.background = background_rgb;
  strcpy(op.text,ch);
  op.textMem=CurMem;
  op.TTY=TTY;
  op.ModeBold=ModeBold;
  op.Rotate=Rotate;
  op.CurMode=CurMode;
  screen_queue_add(screen_queue,op);  
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  // Fixme: come back here and remove direct draw commands.
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
 * screen_redraw()
 */
void screen_redraw(void)
{
  /* screen_clear(); */
  screen_remap_palette();
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
void screen_paint(padPt* Coord)
{
  ScreenOp op;
  op.type=SCREEN_OP_PAINT;
  op.Coord1.x = Coord->x;
  op.Coord1.y = Coord->y;
  op.foreground = foreground_rgb;
  op.background = background_rgb;
  screen_queue_add(screen_queue,op);  
}

