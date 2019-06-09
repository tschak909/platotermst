#include <stdio.h>
#include <string.h>
#include <gem.h>
#include <stdbool.h>

#include "dialog.h"

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


static short max_dialogs = 10;
static short num_dialogs = 0;


static struct dialog_handler **dialog_list;

void init_dialogs() 
{
    static int initialized = 0;

    /* initialize the dialog list */
	if (initialized)
	{
		form_alert(1, "[1][Attempt for multiple initializations |of dialog list.][CANCEL]");
		return;
	}
	dialog_list = malloc(sizeof(struct dialog_handler *) * max_dialogs);
    memset(dialog_list, 0, sizeof(struct dialog_handler *) * max_dialogs);

	initialized = 1;
}

void free_dialogs()
{
	short i;

	for (i = 0; i < num_dialogs; i++)
	{
		free(dialog_list);
		max_dialogs = 0;
		num_dialogs = 0;
	}
}

void add_dialog(struct dialog_handler *dial)
{
	short i;

	if (num_dialogs > max_dialogs - 1)
	{
		struct dialog_handler **new_stack;

		new_stack = malloc(sizeof(struct dialog_handler *) * max_dialogs * 2);

		for (i = 0; i < max_dialogs; i++)
		{
			new_stack[i] = dialog_list[i];
		}
		free(dialog_list);
		dialog_list = new_stack;
		max_dialogs *= 2;
	}
	dialog_list[num_dialogs++] = dial;
}

void delete_dialog(struct dialog_handler *dial)
{
	short i;

	for (i = 0; i < num_dialogs; i++)
	{
		if (dialog_list && dialog_list[i] == dial)
		{
			short j;

            /* FIXME: do whatever is needed to remove the dialog from the screen */

			for (j = i; j < num_dialogs - 1; j++)
			{
				dialog_list[j] = dialog_list[j + 1];
			}
			num_dialogs--;
			break;
		}
	}
	free(dial);
}

struct dialog_handler *from_treeindex(short tree)
{
	short i;

	for (i = 0; i < num_dialogs; i++)
	{
		if (dialog_list[i]->tree == tree)
		{
			return dialog_list[i];
		}
	}
	return NULL;
}

void dialog_do(struct dialog_handler *dial)
{
	short x, y, w, h;
	short exit_obj;
    bool cont = false;

	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);

	form_center(dial->dialog_object, &x, &y, &w, &h);

	form_dial(FMD_GROW, 10, 10, 10, 10, x, y, w, h);
	form_dial(FMD_START, x, y, w, h, x, y, w, h);

    objc_draw(dial->dialog_object, ROOT, MAX_DEPTH, x, y, w, h);
    do
    {
        cont = false;
        exit_obj = form_do(dial->dialog_object, ROOT);

        exit_obj &= 0x7fff;		/* mask out double click bit */

        if ((dial->dialog_object[exit_obj].ob_flags & OF_TOUCHEXIT) &&
                dial->touchexit_callback != NULL)
        {
            /*
             * we do not reset any states here neither do a redraw - this
             * is supposed to be done in the touchexit callback
             */
            cont = dial->touchexit_callback(dial, exit_obj);
        }
    } while (cont);

	form_dial(FMD_FINISH, x, y, w, h, x, y, w, h);
	form_dial(FMD_SHRINK, 10, 10, 10, 10, x, y, w, h);

	wind_update(END_UPDATE);
	wind_update(END_MCTRL);
	
	dial->dialog_object[exit_obj].ob_state &= ~OS_SELECTED;
}

struct dialog_handler *create_dialog(short tree, exit_callback_func *ec, touchexit_callback_func *te)
{
	struct dialog_handler *dial;
	
	dial = malloc(sizeof(struct dialog_handler));

	if (dial == NULL)
	{
		form_alert(1, "[Cannot create dialog][CANCEL]");
		return NULL;
	}

	memset(dial, 0, sizeof(struct dialog_handler));
	dial->tree = tree;
	rsrc_gaddr(R_TREE, tree, &dial->dialog_object);

    /*
     * register callbacks
     */
	dial->exit_callback = ec;
	dial->touchexit_callback = te;
	dial->dialog_do = &dialog_do;

	add_dialog(dial);

	return dial;
}

