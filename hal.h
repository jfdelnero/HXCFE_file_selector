////////////////////////////////////
// FDC I/O                        //
////////////////////////////////////
void init_fdc(int drive);
void deinit_fdc();

int get_start_unit(char * path);
int jumptotrack(unsigned char t);
unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache);
unsigned char writesector(unsigned char sectornum,unsigned char * data);

/////////////////////////////////////
// Keyboard / Joystick / mouse I/O //
/////////////////////////////////////
unsigned char Keyboard();
unsigned char wait_function_key();
unsigned char get_char();
void flush_char();
void disablemousepointer();

////////////////////////////////////
// Screen
////////////////////////////////////
int  init_display(ui_context * ctx);
void print_char8x8(ui_context * ctx, unsigned char * membuffer, unsigned char * font, int col, int line, unsigned char c, int mode);
int  restore_display(ui_context * ctx);

////////////////////////////////////
// System                         //
////////////////////////////////////
void reboot();
void waitsec(int secs);
void waitms(int  ms);
void lockup();

int process_command_line(int argc, char* argv[]);
