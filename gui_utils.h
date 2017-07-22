
#define LEFT_ALIGNED 0x00
#define CENTER_ALIGNED 0x01
#define RIGHT_ALIGNED 0x02
#define DONTPARSE 0x10

int hxc_print(unsigned char mode,int x_pos,int y_pos,char * chaine);
int hxc_printf(unsigned char mode,int x_pos,int y_pos,char * chaine, ...);
int hxc_printf_box(char * chaine, ...);

void save_box();
void restore_box();

void h_line(int y_pos,unsigned short val);
void invert_line(int x_pos,int y_pos);
void invert_line_move(int x_pos,int y_pos_old,int y_pos_new);

void clear_line(int y_pos,unsigned short val);
void box(int x_p1,int y_p1,int x_p2,int y_p2,unsigned short fillval,unsigned char fill);
void init_display_buffer();

unsigned char set_color_scheme(unsigned char color);

#ifdef DEBUG
void dbg_printf(char * chaine, ...);
void print_hex_array(unsigned char * buffer,int size);
#endif

