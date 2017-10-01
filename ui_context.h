typedef struct ui_context_
{
	char filter[32];
	char currentPath[4*256];

	disk_in_drive * disk_ptr;
	cfgfile * cfgfile_ptr;
	unsigned char colormode;
	int page_mode_index;

	int page_number;
	int selectorpos;
	int slotselectorpos;
	int slotselectorpage;

	int config_file_number_max_of_slot;
	int number_of_drive;
	int slots_position;
	int number_of_slots;
	unsigned char filtermode;
	int cfg_file_format_version;
	unsigned char slot_map[512];
	unsigned char change_map[512];

	char FIRMWAREVERSION[16];

	int SCREEN_XRESOL;
	int SCREEN_YRESOL;
	int NUMBER_OF_ENTRIES_ON_DISPLAY;
	int NUMBER_OF_FILE_ON_DISPLAY;

	int screen_txt_xsize;
	int screen_txt_ysize;

	int bootdev;

	int firmware_type;
}ui_context;
