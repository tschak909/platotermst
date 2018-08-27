
#include "keyboard.h"
#include "key.h"
#include "protocol.h"
#include "io.h"
#include "appl.h"
#include <mint/sysbind.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <gem.h>
#include <windom.h>

unsigned char ch;

void keyboard_out(unsigned char platoKey)
{
  if (platoKey==0xff)
    return;
  
  if (platoKey>0x7F)
    {
      Key(ACCESS);
      Key(ACCESS_KEYS[platoKey-0x80]);
      return;
    }
  Key(platoKey);
  return;
}

void keyboard_main(int code, unsigned char shift)
{

  if (TTY)
    {
      keyboard_out_tty(code&0xff);
    }
  else if (shift==0x05 || shift==0x06)
    {
      // Ctrl-Shift pressed
      keyboard_out(ctrl_shift_key_to_pkey[code&0xff]);
    }
  else if (shift==0x08)
    {
      if ((code>>8)==16)
	{
	  ApplWrite( _AESapid, AP_TERM,0,0,0,0,0);
	}
    }
  else
    {
      // no modifiers
      keyboard_out(key_to_pkey[code&0xff]);
    }
}

void keyboard_clear(void)
{
}

void keyboard_out_tty(char ch)
{
  io_send_byte(ch);
}
