
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

unsigned char char_buff[280];

extern padBool FastText; /* protocol.c */
extern unsigned short scalex[];
extern unsigned short scaley[];

extern unsigned char font[];
extern unsigned char fontm23[];
extern unsigned short fontptr[];
extern unsigned char FONT_SIZE_X;
extern unsigned char FONT_SIZE_Y;
extern unsigned short full_screen;
extern unsigned short window_x;
extern unsigned short window_y;

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
  appl_clear_screen();
  screen_queue_dispose(screen_queue);
  screen_queue=screen_queue_create(0,0,0,0,0,NULL,0,NULL);
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

  pxyarray[0]=screen_x(Coord->x);
  pxyarray[1]=screen_y(Coord->y);
  pxyarray[2]=screen_x(Coord->x);
  pxyarray[3]=screen_y(Coord->y);

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

  pxyarray[0]=screen_x(Coord1->x);
  pxyarray[1]=screen_y(Coord1->y);
  pxyarray[2]=screen_x(Coord2->x);
  pxyarray[3]=screen_y(Coord2->y);

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
  short offset; /* due to negative offsets */
  unsigned short x;      /* Current X and Y coordinates */
  unsigned short y;
  unsigned short* px;   /* Pointers to X and Y coordinates used for actual plotting */
  unsigned short* py;
  unsigned char i; /* current character counter */
  unsigned char a; /* current character byte */
  unsigned char j,k; /* loop counters */
  char b; /* current character row bit signed */
  unsigned char width=FONT_SIZE_X;
  unsigned char height=FONT_SIZE_Y;
  unsigned short deltaX=1;
  unsigned short deltaY=1;
  unsigned char mainColor=1;
  unsigned char altColor=0;
  unsigned char *p;
  unsigned char* curfont;
  short pxyarray[4];
  char* chptr;

  // Create copy of character buffer, if queuing up.
  if (queue==TRUE)
    chptr=screen_strndup(ch,count);
  
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

  if (CurMode==ModeRewrite)
    {
      altColor=0;
    }
  else if (CurMode==ModeInverse)
    {
      altColor=1;
    }
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    mainColor=0;
  else
    mainColor=1;

  // bm_setforeground(mainColor);

  x=screen_x((Coord->x&0x1FF));
  y=screen_y((Coord->y)+14&0x1FF);
  
  if (FastText==padF)
    {
      goto chardraw_with_fries;
    }

  /* the diet chardraw routine - fast text output. */
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;
	  
  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  pxyarray[0]=x;
		  pxyarray[1]=y;
		  pxyarray[2]=x;
		  pxyarray[3]=y;
		  v_pline(app.aeshdl,2,pxyarray);
		}

	      ++x;
  	      b<<=1;
  	    }

	  ++y;
	  x-=width;
	  ++p;
  	}

      x+=width;
      y-=height;
    }

  if (queue==true)
    {
      screen_queue_append(screen_queue,SCREEN_QUEUE_CHAR,Coord->x,Coord->y,0,0,chptr,count);
    }

  return;

 chardraw_with_fries:
  if (Rotate)
    {
      deltaX=-abs(deltaX);
      width=-abs(width);
      px=&y;
      py=&x;
    }
    else
    {
      px=&x;
      py=&y;
    }
  
  if (ModeBold)
    {
      deltaX = deltaY = 2;
      width<<=1;
      height<<=1;
    }
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;

	  if (Rotate)
	    {
	      px=&y;
	      py=&x;
	    }
	  else
	    {
	      px=&x;
	      py=&y;
	    }

  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  // bm_setforeground(mainColor);
		  if (ModeBold)
		    {
		      pxyarray[0]=*px+1;
		      pxyarray[1]=*py;
		      pxyarray[2]=*px+1;
		      pxyarray[3]=*py;
		      v_pline(app.aeshdl,2,pxyarray);
		      pxyarray[0]=*px;
		      pxyarray[1]=*py+1;
		      pxyarray[2]=*px;
		      pxyarray[3]=*py+1;
		      v_pline(app.aeshdl,2,pxyarray);
		      pxyarray[0]=*px+1;
		      pxyarray[1]=*py+1;
		      pxyarray[2]=*px+1;
		      pxyarray[3]=*py+1;
		      v_pline(app.aeshdl,2,pxyarray);		      
		    }
		  pxyarray[0]=*px;
		  pxyarray[1]=*py;
		  pxyarray[2]=*px;
		  pxyarray[3]=*py;
		  v_pline(app.aeshdl,2,pxyarray);

		}
	      else
		{
		  if (CurMode==ModeInverse || CurMode==ModeRewrite)
		    {
		      // bm_setforeground(altColor);
		      if (ModeBold)
			{
			  pxyarray[0]=*px+1;
			  pxyarray[1]=*py;
			  pxyarray[2]=*px+1;
			  pxyarray[3]=*py;
			  v_pline(app.aeshdl,2,pxyarray);
			  pxyarray[0]=*px;
			  pxyarray[1]=*py+1;
			  pxyarray[2]=*px;
			  pxyarray[3]=*py+1;
			  v_pline(app.aeshdl,2,pxyarray);
			  pxyarray[0]=*px+1;
			  pxyarray[1]=*py+1;
			  pxyarray[2]=*px+1;
			  pxyarray[3]=*py+1;
			  v_pline(app.aeshdl,2,pxyarray);		      
			}
		      pxyarray[0]=*px;
		      pxyarray[1]=*py;
		      pxyarray[2]=*px;
		      pxyarray[3]=*py;
		      v_pline(app.aeshdl,2,pxyarray);		      
		    }
		}

	      x += deltaX;
  	      b<<=1;
  	    }

	  y+=deltaY;
	  x-=width;
	  ++p;
  	}

      Coord->x+=width;
      x+=width;
      y-=height;
    }

  if (queue==true)
    {
      screen_queue_append(screen_queue,SCREEN_QUEUE_CHAR,Coord->x,Coord->y,0,0,chptr,count);
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

