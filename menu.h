
typedef struct _menu
{
	const char * text;
	int  (*menu_cb)(ui_context * ctx, int event, int xpos, int ypos, int parameter);
	int  cb_parameter;
	struct menu * submenu;
	int align;
}menu;

int enter_menu(ui_context * ctx, const menu * submenu);

enum
{
	MENU_STAYINMENU = 0,
	MENU_LEAVEMENU,
	MENU_REDRAWMENU
};