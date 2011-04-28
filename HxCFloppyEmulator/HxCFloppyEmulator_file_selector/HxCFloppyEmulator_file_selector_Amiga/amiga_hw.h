void init_amiga_fdc(unsigned char drive);
unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache);
unsigned char writesector(unsigned char sectornum,unsigned char * data);
unsigned char Keyboard();
unsigned char wait_function_key();
void jumptotrack(unsigned char t);
void reboot();
void wait_released_key();
unsigned short get_vid_mode();

#define L_INDIAN(var) (((var&0x000000FF)<<24) |((var&0x0000FF00)<<8) |((var&0x00FF0000)>>8) |((var&0xFF000000)>>24))
