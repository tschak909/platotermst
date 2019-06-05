#ifndef _UTIL_H_
#define _UTIL_H_

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

extern short init_util(void);
extern short free_util(void);
extern short open_vwork(short work_out[]);
extern void set_clipping(short handle, short x, short y, short w, short h, short on);
extern void fix_menu(OBJECT *tree);

#endif /* _UTIL_H_ */

