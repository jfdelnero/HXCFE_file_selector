////////////////////////////////////
// FDC I/O                        //
////////////////////////////////////
int init_fdc(int drive);
void deinit_fdc();

int get_start_unit(char * path);
int jumptotrack(unsigned char t);
int readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache);
int writesector(unsigned char sectornum,unsigned char * data);

/////////////////////////////////////
// Keyboard / Joystick / mouse I/O //
/////////////////////////////////////
unsigned char Keyboard();
unsigned char wait_function_key();
unsigned char get_char();
void disablemousepointer();

////////////////////////////////////
// Screen
////////////////////////////////////
int  init_display(ui_context * ctx);
void print_char8x8(ui_context * ctx, int col, int line, unsigned char c, int mode);
void clear_line(ui_context * ctx, int line, int mode);
int  restore_display(ui_context * ctx);

////////////////////////////////////
// System                         //
////////////////////////////////////
void reboot();
void waitsec(int secs);
void waitms(int  ms);
void lockup();

int process_command_line(int argc, char* argv[]);
