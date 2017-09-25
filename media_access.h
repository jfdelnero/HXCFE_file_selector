int media_access_init(int drive);
int setlbabase(unsigned long lba);
int test_floppy_if();
int media_read(uint32 sector, uint8 *buffer, uint32 sector_count);
int media_write(uint32 sector, uint8 *buffer, uint32 sector_count);

enum
{
	INVALID_FIRMWARE = 0,
	HXC_LEGACY_FIRMWARE,
	HXC_CLONE_FIRMWARE,
	CORTEX_FIRMWARE
};
