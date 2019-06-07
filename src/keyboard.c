
#include "keyboard.h"
#include "key.h"
#include "protocol.h"
#include "io.h"
#include <mint/sysbind.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>
#include <gem.h>

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
    unsigned char key=code>>8;
  if (key==0x3b)
    {
      /* MenuEnable(); */
    }
  else if (key==0x44)
    {
      /* appl_form_quit(); */
    }
  else if (shift==0x08)
    {
      if (key==0x23) // ALT-H
	{
	  /* appl_hang_up(); */
	}
    }
  else if (TTY)
    {
      keyboard_out_tty(code&0xff);
    }
  else if (shift==0x05 || shift==0x06)
    {
      // Ctrl-Shift pressed
      keyboard_out(ctrl_shift_key_to_pkey[code&0xff]);
    }
  else
    {
      // no modifiers
      keyboard_out(key_to_pkey[code&0xff]);
    }
}

void keyboard_out_tty(char ch)
{
  io_send_byte(ch);
}
