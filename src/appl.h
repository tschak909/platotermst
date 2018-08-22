
/**
 * Initialize the application context
 */
void applinit(void);

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
