#include <stdbool.h>
#include <gem.h>
#include <windom.h>
#include "protocol.h"
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"

unsigned char already_started=false;

extern padByte splash[];
extern short appl_init_successful;

int main(void)
{
  screen_init();
  applinit();
  terminal_show_greeting();
  terminal_init();
  already_started=true;
  
  if (appl_init_successful==true)
    {
      applmain();
      appldone();
      return 0;
    }
  else
    {
      appldone();
      return -1;
    }
}
