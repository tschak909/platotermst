#ifndef _MENU_H_
#define _MENU_H_

#include <gem.h>
#include <stdint.h>
#include <stdbool.h>

extern short init_menu();
extern short free_menu();
extern short handle_menu(OBJECT *menu, short title, short item);
extern bool is_menu_key(OBJECT *root, short key, short state, short *title, short *item);

#endif /* _MENU_H_ */

