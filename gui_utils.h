
#define LEFT_ALIGNED 0x00
#define CENTER_ALIGNED 0x01
#define RIGHT_ALIGNED 0x02
#define DONTPARSE 0x10


int hxc_print(unsigned char mode,unsigned short x_pos,unsigned short y_pos,char * chaine);
int hxc_printf(unsigned char mode,unsigned short x_pos,unsigned short y_pos,char * chaine, ...);
int hxc_printf_box(char * chaine, ...);
void restore_box();

void h_line(unsigned short y_pos,unsigned short val);
void invert_line(unsigned short x_pos,unsigned short y_pos);
void invert_line_move(unsigned short x_pos,unsigned short y_pos_old,unsigned short y_pos_new);

void clear_line(unsigned short y_pos,unsigned short val);
void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill);
int init_display();
void set_color_scheme(unsigned char color);
