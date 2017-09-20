
typedef struct _menu
{
	const char * text;
	void (*menu_cb)(ui_context * uicontext, int event, int xpos, int ypos);			// Params : context, left/right/sel.
	struct menu * submenu;
	int align;
}menu;

int enter_menu(ui_context * uicontext, const menu * submenu);
