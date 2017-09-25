typedef struct Cortex_cfgfile_
{
	unsigned char undef[22];
	unsigned short number_of_slot;     
	unsigned short slot_index;
}__attribute__((__packed__)) Cortex_cfgfile;

struct Cortex_ShortDirectoryEntry {
	unsigned char name[12];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char longName[41];	// boolean
}__attribute__((__packed__));

extern struct Cortex_DirectoryEntry Cortex_directoryEntry;

typedef struct Cortex_disk_in_drive_
{
	struct ShortDirectoryEntry DirEnt;
}__attribute__((__packed__)) Cortex_disk_in_drive;
