#include <gem.h>
#include <osbind.h>
#include <string.h>
#include "io.h"
#include "protocol.h"
#include "config.h"
#include "window.h"
#include "screen.h"
#include <mint/sysbind.h>
#include <mint/ostruct.h>
#include <gem.h>
#include "util.h"

extern ConfigInfo config;
extern struct window* screen_window;

struct iorec
{
  char *buf;    /* pointer to an array: char buffer[bufsiz] */
  short bufsiz; /* size of the array         */
  short head;   /* index for writing         */
  short tail;   /* index for reading         */
  short low;    /* low water mark, for XON   */
  short high;   /* high water mark, for XOFF */
};
  
typedef struct
{
  struct iorec in;        /*  0 Input buffer record  */
  struct iorec out;       /* 14 Output buffer record */
  char rsr_status;        /* 28 MFP(0x1C) Receiver status */
  char tsr_status;        /* 29 MFP(0x1D) Transmit status */
  char xoff_sent;         /* 30 TRUE if we sent XOFF      */
  char xoff_received;     /* 31 TRUE if we got XOFF       */
  char mode;              /* 32 Bit 0 XON, Bit 1 RTS mode */
  char filler;            /* 33 Unused                    */
} IOREC;


#define IBUFSIZ 8192
#define OBUFSIZ 16

char    st_ibuf[IBUFSIZ];       /* our own input buffer         */
char    st_obuf[OBUFSIZ];       /* and our own output buffer    */

unsigned char io_buffer[8192];
short io_buffer_len;

IOREC   *st_sysr;               /* ptr to system rs232 record   */
IOREC   st_savr;                /* to save system rs232 record  */

IOREC   st_myiorec =
  {
    /* first, an input record */
    st_ibuf, IBUFSIZ, 0, 0, (IBUFSIZ/4), (3*(IBUFSIZ/4)),
    
    /* then an output record */
    st_obuf, OBUFSIZ, 0, 0, 0, 1,
    
    /* and the rsr, tsr, flow control stuff */
    0, 0, 0, 0, 0, 0
  };

extern short vdi_handle;

void io_init(void)
{
  int i;
  // Right now, bare and naive.
  io_configure();

  // Send the init string.
  for (i=0;i<strlen(config.init_str);i++)
    {
      if (config.init_str[i]==0x5F)
	continue;
      io_send_byte(config.init_str[i]);
    }

  // Send CR
  io_send_byte(0x0D);
  
}

void io_configure(void)
{
  Rsconf(config.baud,2,-1,-1,-1,-1);
  while(Bconstat(1))            /* flush existing buffer */
    Bconin(1);
  st_sysr = (IOREC *)Iorec(0);
  st_savr = *st_sysr;             /* Save system buffer   */
  *st_sysr = st_myiorec;          /* Set my io buffer     */
} 

void io_send_byte(unsigned char b)
{
  Bconout(1,b);
}

void io_main(void)
{
  struct PLATOTermWindowData* pd=screen_window->priv;
  
  while (Bconstat(1)==-1)
    {
      pd->platoData[pd->platoLen++]=
	 io_buffer[io_buffer_len++]=(unsigned char)Bconin(1)&0xFF;
    }

  if (pd->platoLen > 16000)
    pd->platoLen=0;
  
  if (io_buffer_len>0)
    {
      graf_mouse(M_OFF,0);
      wind_update(BEG_UPDATE);
      set_clipping(vdi_handle,
		   screen_window->work.g_x,
		   screen_window->work.g_y,
		   screen_window->work.g_w,
		   screen_window->work.g_h,
		   1);
      
      ShowPLATO((padByte *)io_buffer,io_buffer_len);
      io_buffer_len=0;
      set_clipping(vdi_handle,
		   screen_window->work.g_x,
		   screen_window->work.g_y,
		   screen_window->work.g_w,
		   screen_window->work.g_h,
		   0);
      wind_update(END_UPDATE);
      graf_mouse(M_ON,0);
    }
}

/**
 * Hang up
 */
void io_hang_up(void)
{
  Bconout(2,0x07);
  Offgibit(0x10); // Lower DTR...
  /* io_send_byte(0x2B); */
  /* io_send_byte(0x2B); */
  /* io_send_byte(0x2B); */
  /* io_send_byte('A'); */
  /* io_send_byte('T'); */
  /* io_send_byte('H'); */
  /* io_send_byte(0x0D); */
  /* io_send_byte(0x0A); */
  /* Ongibit(0x10);  // and bring it back up. */
}

/**
 * io_done - Called to put the system iorec back in place.
 */
void io_done()
{
  *st_sysr = st_savr;
}
