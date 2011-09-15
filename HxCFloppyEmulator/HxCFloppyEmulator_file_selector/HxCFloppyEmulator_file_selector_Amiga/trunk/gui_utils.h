
int hxc_printf(unsigned char mode,unsigned short x_pos,unsigned short y_pos,char * chaine, ...);
int hxc_printf_box(unsigned char mode,char * chaine, ...);
void restore_box();

void h_line(unsigned short y_pos,unsigned short val);
void invert_line(unsigned short y_pos);
void clear_line(unsigned short y_pos,unsigned short val);
void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill);
int init_display();
void set_color_scheme(unsigned char color);

#define VERSIONCODE "1.6a"
#define DATECODE "15/09/2011"
