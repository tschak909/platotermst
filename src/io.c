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

void io_init(void)
{
  // Right now, bare and naive.
  io_configure();
}

void io_configure(void)
{
  Rsconf(config.baud,2,-1,-1,-1,-1);
  Ongibit(0x10);  // raise DTR.
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
  Offgibit(0x10); // Lower DTR.
}
