
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
  else if ((scan==0x62) && ((shift==0x01) || (shift==0x02)))
    {
      keyboard_out(PKEY_HELP1); // SHIFT + HELP key
    }
  else if (scan==0x62)  // HELP key
    {
      keyboard_out(PKEY_HELP);
    }
  else if ((scan==0x48) && ((shift==0x01) || (shift==0x02))) // UP arrow
    {
      keyboard_out(PKEY_W);
    }
  else if ((scan==0x50) && ((shift==0x01) || (shift==0x02))) // DOWN arrow
    {
      keyboard_out(PKEY_X);
    }
  else if ((scan==0x4B) && ((shift==0x01) || (shift==0x02))) // LEFT arrow
    {
      keyboard_out(PKEY_A);
    }
  else if ((scan==0x4D) && ((shift==0x01) || (shift==0x02))) // RIGHT arrow
    {
      keyboard_out(PKEY_D);
    }
  else if (scan==0x48) // UP arrow
    {
      keyboard_out(PKEY_w);
    }
  else if (scan==0x50) // DOWN arrow
    {
      keyboard_out(PKEY_x);
    }
  else if (scan==0x4B) // LEFT arrow
    {
      keyboard_out(PKEY_a);
    }
  else if (scan==0x4D) // RIGHT arrow
    {
      keyboard_out(PKEY_d);
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
