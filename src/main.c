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

void applinit(void)
{
  int magic_os;
  int mint_os;
  
  if (!vdi_init())
    {
      exit(1);
    }

  magic_os = vq_magx();
  mint_os = vq_mint();
  
}

int main(void)
{
  applinit();
  for (;;)
    EvntWindom( MU_MESAG|MU_TIMER|MU_KEYBD|MU_BUTTON);
  
}
