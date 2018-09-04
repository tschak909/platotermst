/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * terminal.c - Terminal state functions
 */

/* Some functions are intentionally stubbed. */
#pragma warn(unused-param, off)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "terminal.h"
#include "screen.h"
#include "appl.h"

/**
 * ASCII Features to return in Features
 */
#define ASC_ZFGT        0x01
#define ASC_ZPCKEYS     0x02
#define ASC_ZKERMIT     0x04
#define ASC_ZWINDOW     0x08

/**
 * protocol.c externals
 */
extern CharMem CurMem;
extern padBool TTY;
extern padBool ModeBold;
extern padBool Rotate;
extern padBool Reverse;
extern DispMode CurMode;
extern padBool FlowControl;

/**
 * screen.c externals
 */
extern unsigned char CharWide;
extern unsigned char CharHigh;
extern padPt TTYLoc;
extern unsigned char FONT_SIZE_Y;
extern unsigned char already_started;

/**
 * appl.c externals
 */
extern int16_t appl_atari_hi_res;
extern int16_t appl_atari_med_res;      // Are we in Atari Med Res (640x200?)
extern int16_t appl_atari_low_res;      // Are we in Atari Low Res (640x200?)
extern int16_t appl_atari_tt_med_res;   // Are we in Atari TT Med Res (640x480?)

/**
 * terminal_init()
 * Initialize terminal state
 */
void terminal_init(void)
{
  terminal_set_tty();
}

/**
 * terminal_initial_position()
 * Set terminal initial position after splash screen.
 */
void terminal_initial_position(void)
{
  TTYLoc.x=0;
  TTYLoc.y=100; // Right under splashscreen.
}

/**
 * terminal_set_tty(void) - Switch to TTY mode
 */
void terminal_set_tty(void)
{
  if (already_started)
    screen_clear();
  TTY=true;
  ModeBold=padF;
  Rotate=padF;
  Reverse=padF;
  CurMem=M0;
  /* CurMode=ModeRewrite; */
  CurMode=ModeWrite;    /* For speed reasons. */
  CharWide=8;
  CharHigh=16;
  TTYLoc.x = 0;        // leftmost coordinate on screen
  TTYLoc.y = 495;      // Top of screen - one character height
}

/**
 * terminal_set_plato(void) - Switch to PLATO mode
 */
void terminal_set_plato(void)
{
  TTY=false;
  screen_clear();
}

/**
 * terminal_get_features(void) - Inquire about terminal ASCII features
 */
unsigned char terminal_get_features(void)
{
  return ASC_ZFGT; /* This terminal can do Fine Grained Touch (FGT) */
}

/**
 * terminal_get_type(void) - Return the appropriate terminal type
 */
unsigned char terminal_get_type(void)
{
  return 12; /* ASCII terminal type */
}

/**
 * terminal_get_subtype(void) - Return the appropriate terminal subtype
 */
unsigned char terminal_get_subtype(void)
{
  return 1; /* ASCII terminal subtype IST-III */
}

/**
 * terminal_get_load_file(void) - Return the appropriate terminal loadfile (should just be 0)
 */
unsigned char terminal_get_load_file(void)
{
  return 0; /* This terminal does not load its resident from the PLATO system. */
}

/**
 * terminal_get_configuration(void) - Return the terminal configuration
 */
unsigned char terminal_get_configuration(void)
{
  return 0x40; /* Touch panel is present. */
}

/**
 * terminal_get_char_address(void) - Return the base address of the character set.
 */
unsigned short terminal_get_char_address(void)
{
  return 0x3000; /* What the? Shouldn't this be 0x3800? */
}

/**
 * terminal_mem_read - Read a byte of program memory.
 * not needed for our terminal, but must
 * be decoded.
 */
padByte terminal_mem_read(padWord addr)
{
  return (0xFF);
}

/**
 * terminal_mem_load - Write a byte to non-character memory.
 * not needed for our terminal, but must be decoded.
 */
void terminal_mem_load(padWord addr, padWord value)
{
  /* Not Implemented */
}

/**
 * Mode5, 6, and 7 are basically stubbed.
 */
void terminal_mode_5(padWord value)
{ 
}

void terminal_mode_6(padWord value)
{ 
}

void terminal_mode_7(padWord value)
{ 
}

/**
 * terminal_ext_allow - External Input allowed. Not implemented.
 */
void terminal_ext_allow(padBool allow)
{
  /* Not Implemented */
}

/**
 * terminal_set_ext_in - Set which device to get input from.
 * Not implemented
 */
void terminal_set_ext_in(padWord device)
{
}

/**
 * terminal_set_ext_out - Set which device to send external data to.
 * Not implemented
 */
void terminal_set_ext_out(padWord device)
{
}

/**
 * terminal_ext_in - get an external input from selected device.
 * Not implemented.
 */
padByte terminal_ext_in(void)
{
  return 0;
}

/**
 * terminal_ext_out - Send an external output to selected device
 * Not implemented.
 */
void terminal_ext_out(padByte value)
{
}

// Temporary PLATO character data, 8x16 matrix
static unsigned char char_data[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static unsigned char BTAB[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01}; // flip one bit on (OR)
static unsigned char BTAB_5[]={0x08,0x10,0x10,0x20,0x20,0x40,0x80,0x80}; // flip one bit on for the 5x6 matrix (OR)
static unsigned char TAB_0_5[]={0x05,0x05,0x05,0x04,0x04,0x04,0x03,0x03,0x02,0x02,0x01,0x01,0x01,0x00,0x00,0x00};
static unsigned char TAB_0_5i[]={0x00,0x00,0x00,0x01,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x05};

static unsigned char TAB_0_4[]={0x00,0x00,0x01,0x02,0x02,0x03,0x03,0x04}; // return 0..4 given index 0 to 7

static unsigned char PIX_THRESH[]={0x03,0x02,0x03,0x03,0x02, // Pixel threshold table.
				   0x03,0x02,0x03,0x03,0x02,
				   0x02,0x01,0x02,0x02,0x01,
				   0x02,0x01,0x02,0x02,0x01,
				   0x03,0x02,0x03,0x03,0x02,
				   0x03,0x02,0x03,0x03,0x02};

static unsigned char PIX_WEIGHTS[]={0x00,0x00,0x00,0x00,0x00, // Pixel weights
				    0x00,0x00,0x00,0x00,0x00,
				    0x00,0x00,0x00,0x00,0x00,
				    0x00,0x00,0x00,0x00,0x00,
				    0x00,0x00,0x00,0x00,0x00,
				    0x00,0x00,0x00,0x00,0x00};

static unsigned char TAB_0_25[]={0,5,10,15,20,25}; // Given index 0 of 5, return multiple of 5.

static unsigned char pix_cnt;     // total # of pixels
static unsigned char curr_word;   // current word
static unsigned char u,v;       // loop counters

extern unsigned char fontm23[2048];

/**
 * terminal_char_load - Store a character into the user definable
 * character set.
 */

void terminal_char_load(padWord charNum, charData theChar)
{
  if (appl_atari_hi_res==TRUE)
    terminal_char_load_hires(charNum,theChar);
  else if (appl_atari_low_res==TRUE)
    terminal_char_load_lores(charNum,theChar);
  else if (appl_atari_med_res==TRUE)
    terminal_char_load_medres(charNum,theChar);
  else if (appl_atari_tt_med_res==TRUE)
    terminal_char_load_ttmedres(charNum,theChar);
  else
    terminal_char_load_fullres(charNum,theChar);
}

void terminal_char_load_lores(padWord charNum, charData theChar)
{
  // Clear char data. 
  memset(char_data,0,sizeof(char_data));
  memset(PIX_WEIGHTS,0,sizeof(PIX_WEIGHTS));
  memset(&fontm23[charNum*6],0,6);
  
  // Transpose character data.  
  for (curr_word=0;curr_word<8;curr_word++)
    {
      for (u=16; u-->0; )
	{
	  if (theChar[curr_word] & 1<<u)
	    {
	      pix_cnt++;
	      PIX_WEIGHTS[TAB_0_25[TAB_0_5[u]]+TAB_0_4[curr_word]]++;
	      char_data[u^0x0F&0x0F]|=BTAB[curr_word];
	    }
	}
    }

  // Determine algorithm to use for number of pixels.
  // Algorithm A is used when roughly half of the # of pixels are set.
  // Algorithm B is used either when the image is densely or sparsely populated (based on pix_cnt).
  if ((54 <= pix_cnt) && (pix_cnt < 85))
    {
      // Algorithm A - approx Half of pixels are set
      for (u=6; u-->0; )
  	{
  	  for (v=5; v-->0; )
  	    {
  	      if (PIX_WEIGHTS[TAB_0_25[u]+v] >= PIX_THRESH[TAB_0_25[u]+v])
  		fontm23[(charNum*6)+u]|=BTAB[v];
  	    }
  	}
    }
  else if ((pix_cnt < 54) || (pix_cnt >= 85))
    {
      // Algorithm B - Sparsely or heavily populated bitmaps
      for (u=16; u-->0; )
	{
	  for (v=8; v-->0; )
	    {
	      if (char_data[u] & (1<<v))
		{
		  fontm23[(charNum*6)+TAB_0_5i[u]]|=BTAB_5[v];
		}
	    }
	}
    }
}

void terminal_char_load_medres(padWord charNum, charData theChar)
{
  memset(char_data,0,sizeof(char_data));
  
  // load and transpose character data into 8x16 array  
  for (curr_word=0;curr_word<8;curr_word++)
    {
      for (u=16; u-->0; )
	{
	  if (theChar[curr_word] & 1<<u)
	    {
	      char_data[u^0x0F&0x0F]|=BTAB[curr_word];
	    }
	}
    }

  // OR pixel rows together, may not work for this one.
  fontm23[(charNum*6)+0]=char_data[0]|char_data[1]|char_data[2];
  fontm23[(charNum*6)+1]=char_data[3]|char_data[4];
  fontm23[(charNum*6)+2]=char_data[5]|char_data[6]|char_data[7];
  fontm23[(charNum*6)+3]=char_data[8]|char_data[9];
  fontm23[(charNum*6)+4]=char_data[10]|char_data[11]|char_data[12];
  fontm23[(charNum*6)+5]=char_data[13]|char_data[14]|char_data[15];
  
}

void terminal_char_load_hires(padWord charNum, charData theChar)
{
  memset(char_data,0,sizeof(char_data));
  
  // load and transpose character data into 8x16 array  
  for (curr_word=0;curr_word<8;curr_word++)
    {
      for (u=16; u-->0; )
	{
	  if (theChar[curr_word] & 1<<u)
	    {
	      char_data[u^0x0F&0x0F]|=BTAB[curr_word];
	    }
	}
    }

  // OR pixel rows together
  fontm23[(charNum*12)+0]=char_data[0];
  fontm23[(charNum*12)+1]=char_data[1];
  fontm23[(charNum*12)+2]=char_data[2]|char_data[3];
  fontm23[(charNum*12)+3]=char_data[4];
  fontm23[(charNum*12)+4]=char_data[5];
  fontm23[(charNum*12)+5]=char_data[6]|char_data[7];
  fontm23[(charNum*12)+6]=char_data[8];
  fontm23[(charNum*12)+7]=char_data[9];
  fontm23[(charNum*12)+8]=char_data[10]|char_data[11];
  fontm23[(charNum*12)+9]=char_data[12];
  fontm23[(charNum*12)+10]=char_data[13];
  fontm23[(charNum*12)+11]=char_data[14]|char_data[15];
}

void terminal_char_load_ttmedres(padWord charNum, charData theChar)
{
  memset(char_data,0,sizeof(char_data));
  
  // load and transpose character data into 8x16 array  
  for (curr_word=0;curr_word<8;curr_word++)
    {
      for (u=16; u-->0; )
	{
	  if (theChar[curr_word] & 1<<u)
	    {
	      char_data[u^0x0F&0x0F]|=BTAB[curr_word];
	    }
	}
    }

  // OR pixel rows together
  fontm23[(charNum*15)+0]=char_data[0];
  fontm23[(charNum*15)+1]=char_data[1];
  fontm23[(charNum*15)+2]=char_data[2];
  fontm23[(charNum*15)+3]=char_data[3];
  fontm23[(charNum*15)+4]=char_data[4];
  fontm23[(charNum*15)+5]=char_data[5];
  fontm23[(charNum*15)+6]=char_data[6];
  fontm23[(charNum*15)+7]=char_data[7];
  fontm23[(charNum*15)+8]=char_data[8];
  fontm23[(charNum*15)+9]=char_data[9];
  fontm23[(charNum*15)+10]=char_data[10];
  fontm23[(charNum*15)+11]=char_data[11];
  fontm23[(charNum*15)+12]=char_data[12];
  fontm23[(charNum*15)+13]=char_data[13];
  fontm23[(charNum*15)+14]=char_data[14]|char_data[15];  
}

void terminal_char_load_fullres(padWord charnum, charData theChar)
{
  // clear char data
  memset(&fontm23[charnum*FONT_SIZE_Y],0,16);

  // Transpose character data
  for (curr_word=0;curr_word<8;curr_word++)
    {
      for (u=16; u-->0; )
	{
	  if (theChar[curr_word] & 1<<u)
	    {
	      fontm23[(charnum*FONT_SIZE_Y)+u^0x0f&0x0f]|=BTAB[curr_word];
	    }
	}
    }

  // and...that's it, really. :)
  
}
