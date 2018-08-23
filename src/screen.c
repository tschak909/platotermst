
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

unsigned char CharWide=8;
unsigned char CharHigh=16;
padPt TTYLoc;
DrawElement* screen_queue=NULL;

extern padBool FastText; /* protocol.c */
extern unsigned short scalex[];
extern unsigned short scaley[];

extern unsigned char font[];
extern unsigned char fontm23[];
extern unsigned short fontptr[];
extern unsigned char FONT_SIZE_X;
extern unsigned char FONT_SIZE_Y;

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
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  screen_queue=screen_queue_create(0,0,0,0,0,NULL,0,NULL);
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
 * screen_cycle_foreground()
 * Go to the next foreground color in palette
 */
void screen_cycle_foreground(void)
{
}

/**
 * screen_cycle_background()
 * Go to the next background color in palette
 */
void screen_cycle_background(void)
{
}

/**
 * screen_cycle_border()
 * Go to the next border color in palette
 */
void screen_cycle_border(void)
{
}

/**
 * screen_update_colors() - Set the terminal colors
 */
void screen_update_colors(void)
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
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  /* appl_clear_screen(); */
  screen_queue_dispose(screen_queue);
  screen_queue=screen_queue_create(0,0,0,0,0,NULL,0,NULL);
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2, bool queue)
{
  short pxyarray[4];
  
  pxyarray[0]=scalex[Coord1->x];
  pxyarray[1]=scaley[Coord1->y];
  pxyarray[2]=scalex[Coord2->x];
  pxyarray[3]=scaley[Coord2->y];

  // initial naive implementation, draw a bunch of horizontal lines the size of bounding box.

  vsf_interior(app.aeshdl,1); // Solid interior
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      vsf_color(app.aeshdl,0); // white
      vswr_mode(app.aeshdl,0); // White
    }
  else
    {
      vsf_color(app.aeshdl,1); // black
      vswr_mode(app.aeshdl,1); // replace
    }

  v_bar(app.aeshdl,pxyarray);
  
  if (queue==true)
    screen_queue_append(screen_queue,SCREEN_QUEUE_BLOCK_ERASE,Coord1->x,Coord1->y,Coord2->x,Coord2->y,NULL,0);
  
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord, bool queue)
{
  short pxyarray[4];

  pxyarray[0]=Coord->x;
  pxyarray[1]=Coord->y;
  pxyarray[2]=Coord->x;
  pxyarray[3]=Coord->y;

  if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      vsf_color(app.aeshdl,0); // white
      vswr_mode(app.aeshdl,0); // White
    }
  else
    {
      vsf_color(app.aeshdl,1); // black
      vswr_mode(app.aeshdl,1); // replace
    }

  v_pline(app.aeshdl,2,pxyarray);

  if (queue==true)
    screen_queue_append(screen_queue,SCREEN_QUEUE_DOT,Coord->x,Coord->y,0,0,NULL,0);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2, bool queue)
{
  short pxyarray[4];

  pxyarray[0]=scalex[Coord1->x];
  pxyarray[1]=scaley[Coord1->y];
  pxyarray[2]=scalex[Coord2->x];
  pxyarray[3]=scaley[Coord2->y];

   if (CurMode==ModeErase || CurMode==ModeInverse)
    {
      vsl_color(app.aeshdl,0); // white
      vswr_mode(app.aeshdl,0); // White
    }
  else
    {
      vsl_color(app.aeshdl,1); // black
      vswr_mode(app.aeshdl,1); // replace
    }

   v_pline(app.aeshdl,2,pxyarray);
   if (queue==true)
     {
       screen_queue_append(screen_queue,SCREEN_QUEUE_LINE,Coord1->x,Coord1->y,Coord2->x,Coord2->y,NULL,0);
     }
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count, bool queue)
{
  unsigned char* queued_ch;
  short color_index[2]={1,0};
  unsigned char* curfont;
  char offset;
  short x,y;
  unsigned char i;
  MFDB screen_mfdb = {0};
  MFDB char_mfdb;
  unsigned char current_char;
  unsigned char* current_char_ptr;
  short pxyarray[8];
  
  if (queue==true)
    {
      queued_ch=screen_strndup(ch,count);
      screen_queue_append(screen_queue,SCREEN_QUEUE_CHAR,Coord->x,Coord->y,0,0,queued_ch,count);
    }

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
      curfont=fontm23;
      offset=-32;
      break;
    case M3:
      curfont=fontm23;
      offset=32;      
      break;
    }

  // Flip color settings for inverse.
  if (CurMode==ModeInverse)
    {
      color_index[0]=0;
      color_index[1]=1;
    }

  x=scalex[(Coord->x&0x1FF)];
  y=scaley[(Coord->y)+14&0x1FF];

  for (i=0;i<count;++i)
    {
      current_char=*ch;
      ++ch;
      current_char+=offset;
      current_char_ptr=&curfont[fontptr[current_char]];
      
      char_mfdb.fd_addr=current_char_ptr;
      char_mfdb.fd_w=0;
      char_mfdb.fd_h=0;
      char_mfdb.fd_wdwidth=1;
      char_mfdb.fd_stand=0;
      char_mfdb.fd_nplanes=1;
      char_mfdb.fd_r1=char_mfdb.fd_r2=char_mfdb.fd_r3=0;

      pxyarray[0]=0;
      pxyarray[1]=0;
      pxyarray[2]=7;
      pxyarray[3]=11;
      pxyarray[4]=x;
      pxyarray[5]=y;
      pxyarray[6]=x+7;
      pxyarray[7]=x+11;
      x+=FONT_SIZE_X;
      vrt_cpyfm(app.aeshdl,1,pxyarray,&char_mfdb,&screen_mfdb,color_index);
    }
  
  return;
  
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
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
      // screen_block_draw(&scalex[TTYLoc.x],&scaley[TTYLoc.y],&scalex[TTYLoc.x+CharWide],&scaley[TTYLoc.y+CharHigh]);
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
    }
}

/**
 * screen_redraw()
 */
void screen_redraw(void)
{
  DrawElement* cursor = screen_queue;
  while(cursor != NULL)
    {
      screen_next_redraw(cursor);
      cursor=cursor->next;
    }
}
