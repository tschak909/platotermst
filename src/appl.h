#include <gem.h>
#include <windom.h>

#define MAINMENU 0

#define FORM_ABOUT 1
#define MENU_ABOUT 7

#define BUTTON_ABOUT_OK 1

/**
 * Initialize the application context
 */
void applinit(void);

/**
 * Generic form event callback for infobox
 */
void appl_generic_form_event( WINDOW *win);

/**
 * Usefull fonction to get object tree from resource.
 */
OBJECT* appl_get_tree( int index);

/**
 * Application main loop
 */
void applmain(void);

/**
 * Full screen
 */
void appl_fullscreen(void);

/**
 * Clear screen
 */
void appl_clear_screen(void);

/**
 * Restore screen
 */
void appl_restore_screen(void);

/**
 * Return whether full screen is needed.
 */
short appl_get_fullscreen(void);

/**
 * Initialize the off-screen bitmap for terminal.
 */
void appl_terminal_bitmap_init(void);

/**
 * close app about menu
 */
static void appl_about_close(WINDOW *win, int index, int mode, void *data);

/**
 * show app about menu
 */
static void appl_about(WINDOW *null, int index, int title, void *data);

/**
 *	Close resources and cleanly quit application.
 */
static void __CDECL appl_term( WINDOW *win, short buff[8]);

/**
 * Finish application
 */
void appldone(void);
