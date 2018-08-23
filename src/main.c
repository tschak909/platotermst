#include <stdbool.h>
#include <gem.h>
#include "protocol.h"
#include "screen.h"
#include "io.h"
#include "terminal.h"
#include "keyboard.h"

unsigned char already_started=false;

extern padByte splash[];

int main(void)
{
  screen_init();
  applinit();
  applmain();  
}
