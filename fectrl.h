void ui_savereboot(ui_context * uicontext,int preselected_slot);
void ui_save(ui_context * uicontext);
void ui_reboot(ui_context * uicontext);
void ui_config_menu(ui_context * uicontext);
void print_help();
int mount_drive(ui_context * uicontext, int drive);
char save_cfg_file(ui_context * uicontext,unsigned char * sdfecfg_file, int pre_selected_slot);

