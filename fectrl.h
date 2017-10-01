int  ui_loadfilelistpage(ui_context * ctx);
int  ui_savereboot(ui_context * ctx,int preselected_slot);
int  ui_save(ui_context * ctx,int preselected_slot);
void ui_reboot(ui_context * ctx);
void ui_config_menu(ui_context * ctx);
void ui_chgcolor(ui_context * ctx,int color);
void print_help(ui_context * ctx);
int  mount_drive(ui_context * ctx, int drive);
void clear_list(ui_context * ctx);

enum
{
	PAGE_QUITAPP = -1,
	PAGE_FILEBROWSER = 0,
	PAGE_SLOTSLIST,
	PAGE_SETTINGS
};
