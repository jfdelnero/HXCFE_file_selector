typedef struct ui_context_
{
	char filter[32];
	unsigned char currentPath[4*256];

	disk_in_drive * disk_ptr;
	cfgfile * cfgfile_ptr;
	unsigned char colormode;
	int page_mode_index;

	unsigned short page_number;
	short selectorpos;
	unsigned short slotselectorpos;
	unsigned short slotselectorpage;

	unsigned short read_entry;
	unsigned long config_file_number_max_of_slot;
	unsigned long number_of_drive;
	unsigned char filtermode;
	int cfg_file_format_version;
	unsigned char slot_map[512];
	unsigned char change_map[512];
}ui_context;
