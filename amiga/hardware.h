#include "graphx/bmaptype.h"

void init_fdc(int drive);
void deinit_fdc();

int jumptotrack(unsigned char t);
unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache);
unsigned char writesector(unsigned char sectornum,unsigned char * data);
int get_start_unit(char * path);

unsigned char Keyboard();
unsigned char wait_function_key();
unsigned char get_char();
void flush_char();
char *strlwr(char *s);

void reboot();

int init_display();
unsigned short get_vid_mode();
void setvideomode(int mode);
void disablemousepointer();
void display_sprite(unsigned char * membuffer, bmaptype * sprite,int x, int y);
void print_char8x8(unsigned char * membuffer, bmaptype * font,int x, int y,unsigned char c);

void init_timer();

void sleep(int secs);
void waitms(int ms);

#define L_INDIAN(var) (((var&0x000000FF)<<24) |((var&0x0000FF00)<<8) |((var&0x00FF0000)>>8) |((var&0xFF000000)>>24))

int process_command_line(int argc, char* argv[]);

void lockup();
