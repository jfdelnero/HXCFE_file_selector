
typedef struct _menu
{
	const char * text;
	int  (*menu_cb)(ui_context * uicontext, int event, int xpos, int ypos, void * param);
	void * cb_parameter;
	struct menu * submenu;
	int align;
}menu;

int enter_menu(ui_context * uicontext, const menu * submenu);

enum
{
	MENU_STAYINMENU = 0,
	MENU_LEAVEMENU
};