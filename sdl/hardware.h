void init_fdc(unsigned char drive);
int jumptotrack(unsigned char t);
unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache);
unsigned char writesector(unsigned char sectornum,unsigned char * data);
int get_start_unit(char * path);

unsigned char Keyboard();
unsigned char wait_function_key();
unsigned char get_char();
void flush_char();

void reboot();

int init_display();
unsigned short get_vid_mode();
void setvideomode(int mode);
void disablemousepointer();


#ifndef BMAPTYPEDEF
#define BMAPTYPEDEF

typedef  struct _bmaptype
{
   int type;
   int Xsize;
   int Ysize;
   int size;
   int csize;
   unsigned char * data;
}bmaptype;

#endif

void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y);
void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c);

void init_timer();

#define L_INDIAN(var) (var)
#define B_INDIAN(var) (((var&0x000000FF)<<24) |((var&0x0000FF00)<<8) |((var&0x00FF0000)>>8) |((var&0xFF000000)>>24))

int process_command_line(int argc, char* argv[]);

#ifdef WIN32
void sleep(int secs);
#endif
void waitms(int ms);
