int media_access_init(int drive);
int setlbabase(unsigned long lba);
int test_floppy_if();
int media_read(uint32 sector, uint8 *buffer, uint32 sector_count);
int media_write(uint32 sector, uint8 *buffer, uint32 sector_count);
