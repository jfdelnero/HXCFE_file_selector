typedef struct ui_context_
{
	char filter[32];
	unsigned char currentPath[4*256];

	disk_in_drive * disk_ptr;
	cfgfile * cfgfile_ptr;
	unsigned char colormode;
	int page_mode_index;

	unsigned short page_number;
	char  selectorpos;
	short slotselectorpos;
	short slotselectorpage;

	short read_entry;
	unsigned long config_file_number_max_of_slot;
	unsigned char filtermode;
}ui_context;
