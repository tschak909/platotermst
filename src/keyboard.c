
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
  unsigned char scan=code>>8;
  unsigned char key=code&0xFF;
  
  if (scan==0x1c)
    {
      // special case for SHIFT-NEXT
      keyboard_out((shift==0x01 || shift==0x02) ? PKEY_NEXT1 : PKEY_NEXT);
    }
  else if (scan==0x39)
    {
      // SPACE vs BACKSPACE
      keyboard_out((shift==0x01 || shift==0x02) ? PKEY_BACKSPACE : PKEY_SPACE);
    }
  else if ((scan==0x23) && (shift==0x04))
    {
      // special case for CTRL-H (HELP)
      keyboard_out(PKEY_HELP);
    }
  else if ((scan==0x23) && ((shift==0x05) || (shift==0x06)))
    {
      // special case for SHIFT-CTRL-H (SHIFT-HELP)
      keyboard_out(PKEY_HELP1);
    }
  else if ((scan==0x32) && (shift==0x04))
    {
      // special case for CTRL-M (MICRO)
      keyboard_out(PKEY_MICRO);
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
