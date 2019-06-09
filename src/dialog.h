#ifndef _DIALOG_H_
#define _DIALOG_H_

extern void init_dialogs();
extern void free_dialogs();

struct dialog_handler;

typedef short (exit_callback_func)(struct dialog_handler *dial, short exit_button);
typedef bool (touchexit_callback_func)(struct dialog_handler *dial, short exit_obj);

struct dialog_handler
{
    short tree;
    OBJECT *dialog_object;	/* the dialog object */
	short startobject; 		/* for form_do(): start edit object */

    exit_callback_func *exit_callback;
    touchexit_callback_func *touchexit_callback;

    void (*dialog_do)(struct dialog_handler *dial);
    void (*dialog_free)(struct dialog_handler *dial);

    void *priv;
};

extern void add_dialog(struct dialog_handler *dial);
extern void delete_dialog(struct dialog_handler *dial);
extern struct dialog_handler *from_treeindex(short tree);
extern struct dialog_handler *create_dialog(short tree, exit_callback_func *ec, touchexit_callback_func *te);

#endif /* _DIALOG_H_ */
