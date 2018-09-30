#include <gem.h>
#include <windom.h>

#define MAINMENU 0

#define FORM_ABOUT 1
#define FORM_BAUD 2
#define FORM_QUIT 3

#define MENU_ABOUT 7
#define MENU_BAUD_RATE 16
#define MENU_QUIT 17

#define BUTTON_ABOUT_OK 1

#define BUTTON_QUIT_YES 2
#define BUTTON_QUIT_NO 3

#define BUTTON_BAUD_CANCEL 1
#define BUTTON_BAUD_OK     2
#define BUTTON_BAUD_300    3
#define BUTTON_BAUD_1200   4
#define BUTTON_BAUD_2400   5
#define BUTTON_BAUD_4800   6
#define BUTTON_BAUD_9600   7
#define BUTTON_BAUD_19200  8

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
 * show quit form
 */
static void appl_menu_quit(WINDOW *win, int index, int mode, void *data);
static void appl_form_quit(void);

void appl_show_menu(void);
void appl_hide_menu(void);

/**
 * close app about menu
 */
static void appl_about_close(WINDOW *win, int index, int mode, void *data);

/**
 * show app about menu
 */
static void appl_menu_about(WINDOW *null, int index, int title, void *data);

/**
 * Show baud rate form
 */
void appl_form_baud(WINDOW* win);

/**
 * show app baud form
 */
static void appl_menu_baud(WINDOW *null, int index, int title, void *data);

/**
 *	Close resources and cleanly quit application.
 */
static void __CDECL appl_term( WINDOW *win, short buff[8]);

/**
 * Show ready prompt
 */
void appl_show_ready(void);

/**
 * Finish application
 */
void appldone(void);
