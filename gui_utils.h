
#define LEFT_ALIGNED 0x01
#define CENTER_ALIGNED 0x02
#define RIGHT_ALIGNED 0x04
#define INVERTED 0x08
#define DONTPARSE 0x10

int hxc_print(ui_context * ctx,unsigned char mode,int col,int line,char * chaine);
int hxc_printf(ui_context * ctx,unsigned char mode,int col,int line,char * chaine, ...);
int hxc_printf_box(ui_context * ctx,char * chaine, ...);

void invert_line(ui_context * ctx, int line);

void init_display_buffer(ui_context * ctx);
unsigned char set_color_scheme(unsigned char color);

char* stristr( const char* str1, const char* str2 );

#ifdef DEBUG
void dbg_printf(char * chaine, ...);
void print_hex_array(unsigned char * buffer,int size);
#endif

