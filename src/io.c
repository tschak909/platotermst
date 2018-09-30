#include <gem.h>
#include <osbind.h>

#include "io.h"
#include "protocol.h"
#include "config.h"
#include <mint/sysbind.h>
#include <mint/ostruct.h>

extern ConfigInfo config;

void io_init(void)
{
  // Right now, bare and naive.
  io_configure();
}

void io_configure(void)
{
  Rsconf(config.baud,2,-1,-1,-1,-1);
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
  unsigned char ch;
  if (Bconstat(1)==-1)
    {
      ch=(unsigned char)Bconin(1)&0xFF;
      ShowPLATO(&ch,1);
    }
}

void io_recv_serial(void)
{  
}

void io_done()
{
}
