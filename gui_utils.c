/*
//
// Copyright (C) 2009-2016 Jean-François DEL NERO
//
// This file is part of the HxCFloppyEmulator file selector.
//
// HxCFloppyEmulator file selector may be used and distributed without restriction
// provided that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator file selector is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator file selector; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "graphx/data_bmp_font8x8_bmp.h"
#include "graphx/data_bmp_hxc2001_smalllogo_bmp.h"

#include "msg_txt.h"

#include "gui_utils.h"

#include "conf.h"

#include "hardware.h"

#include "version.h"

extern char FIRMWAREVERSION[16];

extern unsigned char * screen_buffer;
extern unsigned short SCREEN_XRESOL;
extern unsigned short SCREEN_YRESOL;
unsigned char NUMBER_OF_FILE_ON_DISPLAY;

void print_str(unsigned char * membuffer,char * buf,int maxsize,int x_pos,int y_pos,int linefeed)
{
	int i;
	int x_offset,y_offset;

	x_offset = x_pos;
	y_offset = y_pos;

	#ifdef DEBUG
	dbg_printf("UIOut x:%d y:%d %s\n",y_pos,y_pos,buf);
	#endif

	i = 0;
	while(buf[i] && i < maxsize)
	{
		if(x_offset<=(SCREEN_XRESOL-8))
		{
			if(buf[i] == '\n' && linefeed)
			{
				x_offset = x_pos;
				y_offset += 8;
			}
			else
			{
				print_char8x8(membuffer,bitmap_font8x8_bmp,x_offset,y_offset,buf[i]);
				x_offset=x_offset+8;
			}
		}
		i++;
	}
}

int hxc_print(unsigned char mode,int x_pos,int y_pos,char * string)
{
	int line_size,i,linenb;
	int x_offset;
	int linefeed;

	if(mode&DONTPARSE)
		linefeed = 0;
	else
		linefeed = 1;

	switch(mode & 0xF)
	{
		case LEFT_ALIGNED: // Left aligned
			print_str(screen_buffer,string,MAXTXTSIZE,x_pos,y_pos,linefeed);
		break;
		case CENTER_ALIGNED: // Center aligned
		case RIGHT_ALIGNED: // Right aligned
			linenb = 0;
			i = 0;
			while(string[i])
			{
				line_size = 0;
				while( string[i + line_size] != '\n' && string[i + line_size] != 0)
				{
					line_size++;
				}

				x_offset = (SCREEN_XRESOL-(line_size*8));
				if(mode == CENTER_ALIGNED)
					x_offset /= 2;

				print_str(screen_buffer,&string[i],line_size,x_offset,y_pos + (linenb*8),linefeed);

				if(string[i + line_size] == '\n')
					i += (line_size + 1);
				else
					i += line_size;

				linenb++;
			}
		break;
	}

	return 0;
}

int hxc_printf(unsigned char mode,int x_pos,int y_pos,char * chaine, ...)
{
	char temp_buffer[MAXTXTSIZE];

	va_list marker;
	va_start( marker, chaine );

#if ( _MSC_VER >= 1100 && _MSC_VER <= 1200 ) // VC 6 doesn't have vsnprintf.
	vsprintf(temp_buffer,chaine,marker);
#else
	vsnprintf(temp_buffer,MAXTXTSIZE,chaine,marker);
#endif

	va_end( marker );

	hxc_print(mode,x_pos,y_pos,temp_buffer);

	return 0;
}

void clear_line(int y_pos,unsigned short val)
{
	int i;
	for(i=0;i<8;i++)
		h_line(y_pos+i,val);
}

int hxc_printf_box(char * chaine, ...)
{
	char temp_buffer[1024];
	int str_size;
	int i;
	va_list marker;

	save_box();

	va_start( marker, chaine );

#if ( _MSC_VER >= 1100 && _MSC_VER <= 1200 ) // VC 6 doesn't have vsnprintf.
	vsprintf(temp_buffer,chaine,marker);
#else
	vsnprintf(temp_buffer,1024,chaine,marker);
#endif

	str_size=strlen(temp_buffer) * 8;
	str_size=str_size+(4*8);

	for(i=0;i< str_size;i=i+8)
	{
		print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80-8,8);
	}
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80-8,3);
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80-8,2);

	for(i=0;i< str_size;i=i+8)
	{
		print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80,' ');
	}

	print_str(screen_buffer,temp_buffer,MAXTXTSIZE,((SCREEN_XRESOL-str_size)/2)+(2*8),80,0);
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80,7);
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80,6);

	for(i=0;i< str_size;i=i+8)
	{
		print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+i,80+8,9);
	}
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2)+(i-8),80+8,5);
	print_char8x8(screen_buffer,bitmap_font8x8_bmp,((SCREEN_XRESOL-str_size)/2),80+8,4);

	va_end( marker );

	return 0;
}

void init_display_buffer()
{
	// HxC2001 logo
	display_sprite(screen_buffer, bitmap_hxc2001_smalllogo_bmp,
	                                    (SCREEN_XRESOL-bitmap_hxc2001_smalllogo_bmp->Xsize),
	                                    (SCREEN_YRESOL-bitmap_hxc2001_smalllogo_bmp->Ysize));

	// Horizontal separator lines
	h_line(0,0xFFFF);
	h_line(9,0xFFFF);
	h_line(SCREEN_YRESOL-(bitmap_hxc2001_smalllogo_bmp->Ysize + 1),0xFFFF);

	// Footprint : Current software / firmware version and title
	hxc_printf(LEFT_ALIGNED,0,SCREEN_YRESOL - ( 8 + 2 ),"FW Ver %s",FIRMWAREVERSION);
	hxc_print(CENTER_ALIGNED,0,SCREEN_YRESOL - ( 8 + 2 ),(char*)title_msg);
	hxc_print(LEFT_ALIGNED,0,CURDIR_Y_POS, (char*)cur_folder_msg);

	hxc_print(CENTER_ALIGNED,0,HELP_Y_POS+8, (char*)startup_msg);

	NUMBER_OF_FILE_ON_DISPLAY = ( (SCREEN_YRESOL - (bitmap_hxc2001_smalllogo_bmp->Ysize + 1 ) ) - 10 ) / 8;
}


#ifdef DEBUG

void print_hex_buffer(unsigned char * buffer, int size)
{
	int c,i;
	int x,y;

	c=0;

	x=0;
	y=0;
	for(i=0;i<size;i++)
	{
		x=((c & 0xF)*24);
		hxc_printf(LEFT_ALIGNED,x,y,"%.2X ", buffer[i]);
		c++;
		if(!(c&0xF))
		{
			y=y+9;
		}
	}

	c=0;

	x=0;
	y=0;

	for(i=0;i<size;i++)
	{
		x=((c & 0xF)*8)+384+8;
		if(
			(buffer[i]>='a' && buffer[i]<='z') ||
			(buffer[i]>='A' && buffer[i]<='Z') ||
			(buffer[i]>='0' && buffer[i]<='9')
			)
		{
			hxc_printf(LEFT_ALIGNED,x,y,"%c", buffer[i]);
		}
		else
		{
			hxc_print(LEFT_ALIGNED,x,y,".");
		}
		c++;
		if(!(c&0xF))
		{
			y=y+9;
		}
	}
}

void print_hex_array(unsigned char * buffer,int size)
{
	int i,j;
	int offset;

	for( i = 0; i < (6+(16*3)+1+16) ; i++ )
	{
		dbg_printf("-",i);
	}
	dbg_printf("\n");

	dbg_printf("       ");
	for( i = 0; i < 16 ; i++ )
	{
		dbg_printf(" %X",i);
		if(i!=15)
			dbg_printf(" ");
	}

	offset = 0;
	while ( offset < size )
	{
		if( !(offset&0xF) )
		{
			if(offset)
			{
				dbg_printf(" ");
				for(i=0;i<16;i++)
				{
					if( ( buffer[ offset - 16 + i ] >=' ' && buffer[ offset - 16 + i ] <= 126 ) )
					{
						dbg_printf("%c",buffer[ offset - 16 + i ]);
					}
					else
					{
						dbg_printf("%c",'.');
					}
				}
			}

			dbg_printf("\n%.4X :",offset);
		}

		dbg_printf(" %.2X",buffer[offset]);
		offset++;
	}

	if( offset & 0xF )
		j = offset & 0xF;
	else
		j = 16;

	for( i = j ;i<16 ;i++)
		dbg_printf(" --");

	dbg_printf(" ");
	for( i = 0; i < j; i++ )
	{
		if( ( buffer[ offset - j + i ] >=' ' && buffer[ offset - j + i ] <= 126 ) )
		{
			dbg_printf("%c",buffer[ offset - j + i ]);
		}
		else
		{
			dbg_printf("%c",'.');
		}
	}

	dbg_printf("\n");

	for( i = 0; i < (6+(16*3)+1+16) ; i++ )
	{
		dbg_printf("-",i);
	}
	dbg_printf("\n");

}

#endif
