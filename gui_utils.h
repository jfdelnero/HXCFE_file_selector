
#define LEFT_ALIGNED 0x00
#define CENTER_ALIGNED 0x01
#define RIGHT_ALIGNED 0x02
#define DONTPARSE 0x10

int hxc_print(ui_context * ctx,unsigned char mode,int x_pos,int y_pos,char * chaine);
int hxc_printf(ui_context * ctx,unsigned char mode,int x_pos,int y_pos,char * chaine, ...);
int hxc_printf_box(ui_context * ctx,char * chaine, ...);

void h_line(ui_context * ctx,int y_pos,unsigned short val);
void invert_line(ui_context * ctx,int x_pos,int y_pos);

void clear_line(ui_context * ctx,int y_pos,unsigned short val);
void box(ui_context * ctx,int x_p1,int y_p1,int x_p2,int y_p2,unsigned short fillval,unsigned char fill);
void init_display_buffer(ui_context * ctx);
unsigned char set_color_scheme(unsigned char color);

#ifdef DEBUG
void dbg_printf(char * chaine, ...);
void print_hex_array(unsigned char * buffer,int size);
#endif

