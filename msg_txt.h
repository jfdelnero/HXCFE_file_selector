#define MAXTXTSIZE 2048

extern const char startup_msg[];
extern const char help_scr1_msg[];
extern const char help_scr2_msg[];
extern const char help_scr3_msg[];

extern const char cur_folder_msg[];
extern const char save_msg[];
extern const char save_and_restart_msg[];
extern const char reboot_msg[];
extern const char title_msg[];
extern const char command_menu_msg[];

typedef struct pagedesc_
{
	const char * txt;
	int align;
}pagedesc;

extern const pagedesc help_pages[];