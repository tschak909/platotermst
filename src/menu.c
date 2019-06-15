#include <gem.h>
#include <osbind.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "window.h"
#include "global.h"
#include "util.h"
#include "menu.h"
#include "resource.h"
#include "dialog.h"
#include "screen.h"
#include "io.h"

extern struct dialog_handler* about_dialog;
extern void init_prefs_dialog(void);

//#define DEBUG
#ifdef DEBUG
/*
 * Since writing directly somewhere to the screen would distort GEM, the escape sequences in debug_printf()
 * position the cursor on line 30, column 0, clear to end of line and write the debug message.
 * Make sure you don't add a newline after the message or the screen will be clobbered.
 * This way we have at least one single line to display diagnostic output.
 */
#define debug_printf(format, arg...) do { printf("\033Y\36 \33lDEBUG (%s): " format, __FUNCTION__, ##arg); } while (0)
#else
#define debug_printf(format, arg...) do { ; } while (0)
#endif /* DEBUG */


extern struct dialog_handler *about_dialog;
extern struct dialog_handler *prefs_dialog;
extern struct dialog_handler *keys_dialog;
extern struct dialog_handler *micro_dialog;

static char *fileerr;

static _KEYTAB *__menu_keys = NULL;
/*
 * init the menu module
 */
short init_menu()
{
    short res;

    res = rsrc_gaddr(R_TREE, 0, &gl_menu);
    if (res != 0)
    {
        /* fix_menu(gl_menu); */
        menu_bar(gl_menu, 1);
    }

    __menu_keys = Keytbl((void *) -1, (void *) -1, (void *) -1);

    return res;
}

short free_menu()
{
    return 0;
}

static char path[128] = "C:\\*.*";
static char name[32] = "";

extern char nkc_toupper(char c);    /* external assembly */
void nkcc_str_toupper (char *str)
{
    while (*str != '\0')
    {
        *str = nkc_toupper(*str);
        str++;
    }
}

static inline bool check_item_key(char *str, short kstate, short kreturn)
{
    bool found = false;
    short len;
    short i;
    short r;
    short ks;
    char s[50];
    char ascii;
    unsigned char scan;

    /* do not make shift keys differences */
    if (kstate & K_RSHIFT)
        kstate |= K_LSHIFT;

    /* look only to shift/alt/ctrl modifiers */
    kstate &= K_LSHIFT | K_CTRL | K_ALT;

    scan = (kreturn & 0xff00) >> 8;

    if (scan > 127)
        return false;

    if (scan >= 0x78 && scan < 0x80)
        ascii = __menu_keys->caps[scan - 0x76];
    else
        ascii = __menu_keys->caps[scan];

    if (str[0] != '\0')
    {
        len = (int) strlen(str);
        i = len - 1;
        r = 0;
        while (str[i] == ' ')
        {
            i--;
            r++;
        }
        if (r < 2)
        {
            str[len - r] = '\0';    /* sweep trailing blanks */

            while ((i >= 0) && (str[i] != ' '))
                i--;
            strcpy (s, str + i + 1);
            nkcc_str_toupper(s);
            ks = 0;
            i = 0;
            found = false;
            while (!found)
            {
                switch (s[i])
                {
                    case '^':
                        ks |= K_CTRL;
                        i++;
                        break;
                    case '\7':
                        ks |= K_ALT;
                        i++;
                        break;
                    case '\1':
                        ks |= K_LSHIFT;
                        i++;
                        break;
                    default:
                        found = true;
                }
            }

            found = false;
            if (ks == kstate)   /* ensure special keys match */
            {
                if (s[i + 1] == '\0')   /* only one single character */
                {
                    if (s[i] == 0x1B && scan == 1)  /* ^[ */
                        found = true;
                    else if (s[i] == ascii)
                        found = true;
                }
                else if ((strcmp (s + i, "DEL") == 0)
                         && (scan == 83))
                    found = true;
                else if ((strcmp (s + i, "ESC") == 0)
                         && (scan == 1))
                    found = true;
                else if ((strcmp (s + i, "HELP") == 0)
                         && (scan == 98))
                    found = true;
                else if ((strcmp (s + i, "HOME") == 0)
                         && (scan == 71))
                    found = true;
                else if ((strcmp (s + i, "INS") == 0)
                         && (scan == 82))
                    found = true;
                else if ((strcmp (s + i, "TAB") == 0)
                         && (scan == 15))
                    found = true;
                else if ((strcmp (s + i, "UNDO") == 0)
                         && (scan == 97))
                    found = true;
                else if ((s[i] == 'F') && (s[i + 1] >= '1') && (s[i + 1] <= '9'))
                {
                    if (s[i + 2] == '0' && scan == 68)  /* F10 */
                        found = true;
                    else
                    {
                        if (ks == K_LSHIFT || ks == K_RSHIFT)
                        {
                            if ((scan - 25) == (s[i + 1] + 10))
                                found = true;
                        }
                        else
                        {
                            if (scan == (s[i + 1] + 10))
                                found = true;
                        }
                    }
                }
            }
        }
    }
    return found;
}

/*
 * iterate throug a given menu tree to check if one of the menu accelerators
 * matches a pressed key (key, state)
 *
 * return true if there is a match. In that case, title and item are valid
 * OBJECTs corresponding to the menu title and item where there was a match.
 *
 * From the Atari Compendium:
 *
 * The ROOT object is a G_IBOX and should be set to the same width and height of the screen.
 * It has two children, the BAR object and the DROPDOWNS object. The BAR object is a G_BOX which should
 * be the width of the screen and the height of the system font plus two pixels for a border line. The
 * DROPDOWNS object is a G_IBOX and should be of a size large enough to encompass all of the drop-down
 * menu boxes.
 * The BAR object has one child, the ACTIVE object, it should be the width of the screen and the height
 * of the system font. It has as many G_TITLE children as there are menu titles.
 * The DROPDOWNS object has the same number of G_BOX child objects as the ACTIVE object has G_TITLE
 * children. Each box must be high enough to support the number of G_STRING menu items and wide enough
 * to support the longest item. Each G_BOX must be aligned so that it falls underneath its
 * corresponding G_TITLE. In addition, each G_STRING menu item should be the same length as its parent
 * G_BOX object.
 * Each G_STRING menu item should be preceded by two spaces. Each G_TITLE should be preceded and followed
 * by one space. The first G_BOX object should appear under a G_TITLE object named 'Desk' and should
 * contain eight children. The first child G_STRING is application defined (it usually leads to the
 * 'About...' program credits), the second item should be a disabled separator ('-----------')
 * line. The next six items are dummy objects used by the AES to display program and desk accessory titles.
 */
bool is_menu_key(OBJECT *root, short key, short state, short *title, short *item)
{
    bool result = false;
    short bar_index = 0;
    short active_index;
    short dropdowns_index = 0;
    short curr_title;
    short curr_menu;
    short curr_item;

    if (root->ob_type == G_IBOX)
    {
        debug_printf("root object is G_IBOX");
        if (root[root->ob_head].ob_type == G_BOX)
        {
            /* if the first child is G_BOX, this is the bar object... */
            debug_printf("found BAR object");
            bar_index = root->ob_head;
            dropdowns_index = root[bar_index].ob_next;

            if (root[dropdowns_index].ob_type == G_IBOX)
            {
                /* ...and the second child is the dropdowns object */
                debug_printf("found DROPDOWNS object");
            }
        }
        else if (root[root->ob_head].ob_type == G_IBOX)
        {
            /* else its the other way round */
            debug_printf("found DROPDOWNS object");
            dropdowns_index = root->ob_head;

            bar_index = root[dropdowns_index].ob_next;
            if (root[bar_index].ob_type == G_BOX)
            {
                debug_printf("found BAR object");
            }
        }
        else
        {
            /* there is something wrong */
            debug_printf("root->ob_next neither G_BOX nor G_IBOX, but %s",
                         ob_type_to_str(root[root->ob_next].ob_type));
            return false;
        }
    }

    active_index = root[bar_index].ob_head;     /* first child of the bar object is the active object that contains the menu title objects */
    curr_menu = root[dropdowns_index].ob_head;  /* dropdown menus */

    curr_title = root[active_index].ob_head;

    for (curr_title = root[active_index].ob_head; curr_title != active_index; curr_title = root[curr_title].ob_next)
    {
        /* iterate through active's children */

        debug_printf("title \"%d\" with type \"%s\" (%s)",
                     curr_title, ob_type_to_str(root[curr_title].ob_type), root[curr_title].ob_spec.free_string);

        for (curr_item = root[curr_menu].ob_head; curr_item != curr_menu; curr_item = root[curr_item].ob_next)
        {
            char menu_str[80];
            if (root[curr_item].ob_type == G_STRING || root[curr_item].ob_type == G_BUTTON)
            {
                strcpy(menu_str, root[curr_item].ob_spec.free_string);
                if (check_item_key(menu_str, state, key))
                {
                    debug_printf("%s matched.", menu_str);
                    *title = curr_title;
                    *item = curr_item;
                    return true;
                }
                else
                {
                    debug_printf("no match on %s.", menu_str);
                }
            }
        }
        curr_menu = root[curr_menu].ob_next;
    }

    return result;
}

short handle_menu(OBJECT *menu, short title, short item)
{
    short res;

    res = true;
    switch (title)
      {
      case 3:
	switch(item)
	  {
	  case 8:
	    about_dialog->dialog_do(about_dialog);
	    break;
	  }
	break;
      case 4:
	switch(item)
	  {
	  case 17:
	    init_prefs_dialog();
	    prefs_dialog->dialog_do(prefs_dialog);
	    break;
	  case 18:
	    io_hang_up();
	    break;
	  case 19:
	    screen_restore_vdi_palette();
	    quit=true;
	    break;
	  }
	break;
      case 5:
	switch(item)
	  {
	  case 21:
	    keys_dialog->dialog_do(keys_dialog);
	    break;
	  case 22:
	    micro_dialog->dialog_do(micro_dialog);
	    break;
	  }
	break;
      }

    return res;
    
    /* switch (title) */
    /* { */
    /*     case MMENU_ACC:	/\* accessory menu *\/ */
    /*         switch (item) */
    /*         { */
    /*             case MMENU_ABOUT:	/\* "about"-menu *\/ */
    /*                 { */
    /*                 } */
    /*                 break; */
    /*         } */

    /*     case MMENU_FILE: */
    /*         switch (item) */
    /*         { */
    /*             struct window *wi; */

    /*             case MMENU_OPEN: */
    /*                 wi = create_textwindow(TEXTWINDOW_WINELEMENTS, "Text Window"); */
    /*                 if (wi != NULL) */
    /*                 { */
    /*                     short but; */
    /*                     char fullpath[255]; */

    /*                     wind_update(BEG_MCTRL); */
    /*                     if (fsel_input(path, name, &but) && but == 1) */
    /*                     { */
    /*                         char *res; */
    /*                         char *bs; */

    /*                         bs = strrchr(path, '\\'); */
    /*                         if (bs != NULL) */
    /*                         { */
    /*                             *(bs + 1) = '\0'; */
    /*                             strcpy(fullpath, path); */
    /*                             strcat(fullpath, name); */
    /*                             res = textwindow_load(wi, fullpath); */

    /*                             if (res != NULL) */
    /*                             { */
    /*                                 strcpy(wi->name, fullpath); */
    /*                                 wind_set_str(wi->handle, WF_NAME, wi->name); */

    /*                                 open_window(wi, window_open_pos_x, window_open_pos_y, MIN_WIDTH, MIN_HEIGHT); */
    /*                                 window_open_pos_x += 10; */
    /*                                 window_open_pos_y += 10; */
    /*                                 do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h); */
    /*                             } */
    /*                             else */
    /*                             { */
    /*                                 form_alert(1, fileerr); */
    /*                                 return -1; */
    /*                             } */
    /*                         } */
    /*                     } */
    /*                     wind_update(END_MCTRL); */
    /*                 } */
    /*                 break; */

    /*             case MMENU_CLOSE: */
    /*                 if ((wi = top_window())) */
    /*                 { */
    /*                     delete_window(wi); */
    /*                 } */
    /*                 break; */

    /*             case MMENU_QUIT: */
    /*                 quit = true; */
    /*                 debug_printf("quit selected"); */
    /*                 break; */

    /*             default: */
    /*                 res = false; /\* not handled *\/ */
    /*                 /\* no break *\/ */
    /*         } /\* switch(item) *\/ */
    /*         break; */

    /*     case MMENU_EDIT: */
    /*         switch (item) */
    /*         { */
    /*             case MMENU_SETTINGS:	/\* Preferences *\/ */
    /*                 /\* handle preferences dialog *\/ */
    /*                 if (prefs_dialog && prefs_dialog->dialog_do) */
    /*                 { */
    /*                     prefs_dialog->dialog_do(prefs_dialog); */
    /*                 } */
    /*                 ; */
    /*                 break; */

    /*             default: */
    /*                 ; */
    /*         } */
    /*         break; */

    /*     default: */
    /*         res = false; /\* not handled *\/ */
    /*         /\* no break *\/ */
    /* } /\* switch(title) *\/ */

    /* return res; */
}
