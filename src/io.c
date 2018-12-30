#include <gem.h>
#include <osbind.h>

#include "io.h"
#include "protocol.h"
#include "config.h"
#include <mint/sysbind.h>
#include <mint/ostruct.h>

extern ConfigInfo config;

static unsigned char io_buffer[4096];
static unsigned short io_buffer_size;

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


#define IBUFSIZ 16384
#define OBUFSIZ 16

char    st_ibuf[IBUFSIZ];       /* our own input buffer         */
char    st_obuf[OBUFSIZ];       /* and our own output buffer    */

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

void io_init(void)
{
  // Right now, bare and naive.
  io_configure();
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

/**
 * Not needed for now, remove.
 */
void io_init_funcptrs(void)
{
}

void io_open(void)
{  
}

void io_send_byte(unsigned char b)
{
  Bconout(1,b);
}

void io_main(void)
{
  while (Bconstat(1)==-1)
    {
      io_buffer[io_buffer_size++]=(unsigned char)Bconin(1)&0xFF;
    }

  if (io_buffer_size>0)
    {
      ShowPLATO((padByte *)io_buffer,io_buffer_size);
      io_buffer_size=0;
    }

  /* if (Bconstat(1)==-1) */
  /*   { */
  /*     ch=(unsigned char)Bconin(1)&0xFF; */
  /*     ShowPLATO(&ch,1); */
  /*   } */
}

void io_recv_serial(void)
{  
}

/**
 * Hang up
 */
void io_hang_up(void)
{
  Bconout(2,0x07);
  Offgibit(0x10); // Lower DTR...
  io_send_byte(0x2B);
  io_send_byte(0x2B);
  io_send_byte(0x2B);
  usleep(3000000);  // for a moment.
  io_send_byte('A');
  io_send_byte('T');
  io_send_byte('H');
  io_send_byte(0x0D);
  io_send_byte(0x0A);
  Ongibit(0x10);  // and bring it back up.
}


void io_done()
{
  *st_sysr = st_savr;
}
