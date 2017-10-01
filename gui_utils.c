/*
//
// Copyright (C) 2009-2017 Jean-François DEL NERO
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdarg.h>

#include "cfg_file.h"

#include "msg_txt.h"

#include "conf.h"
#include "ui_context.h"

#include "gui_utils.h"

#include "conf.h"

#include "hal.h"

#include "version.h"

void print_str(ui_context * ctx,char * buf,int maxsize,int col,int line,int linefeed,int mode)
{
	int i;
	int x_offset,y_offset;

	x_offset = col;
	y_offset = line;

	#ifdef DEBUG
	dbg_printf("UIOut x:%d y:%d %s\n",col,line,buf);
	#endif

	#ifdef SDLHOST
	if(strlen(buf)>2)
	{
		printf("%s\n",buf);
	}
	#endif

	i = 0;
	while(buf[i] && i < maxsize)
	{
		if( (x_offset*8) <=(ctx->SCREEN_XRESOL-8))
		{
			if(buf[i] == '\n' && linefeed)
			{
				x_offset = line;
				y_offset++;
			}
			else
			{
				print_char8x8(ctx,x_offset,y_offset,buf[i],mode);
				x_offset++;
			}
		}
		i++;
	}
}

int hxc_print(ui_context * ctx,unsigned char mode,int col,int line,char * string)
{
	int line_size,i,linenb;
	int x_offset;
	int linefeed;

	if(mode&DONTPARSE)
		linefeed = 0;
	else
		linefeed = 1;

	switch( ( mode & (LEFT_ALIGNED|CENTER_ALIGNED|RIGHT_ALIGNED)) )
	{
		case LEFT_ALIGNED: // Left aligned
			print_str(ctx,string,MAXTXTSIZE,col,line,linefeed,mode);
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

				x_offset = ctx->screen_txt_xsize - line_size;
				if(mode & CENTER_ALIGNED)
					x_offset /= 2;

				print_str(ctx,&string[i],line_size,x_offset,line + linenb,linefeed,mode);

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

int hxc_printf(ui_context * ctx,unsigned char mode,int col,int line,char * chaine, ...)
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

	hxc_print(ctx,mode,col,line,temp_buffer);

	return 0;
}

int hxc_printf_box(ui_context * ctx,char * chaine, ...)
{
	char temp_buffer[1024];
	int str_size;
	int i;
	va_list marker;

	va_start( marker, chaine );

#if ( _MSC_VER >= 1100 && _MSC_VER <= 1200 ) // VC 6 doesn't have vsnprintf.
	vsprintf(temp_buffer,chaine,marker);
#else
	vsnprintf(temp_buffer,1024,chaine,marker);
#endif

	str_size = strlen(temp_buffer);
	str_size += 4;

	// Upper bar
	for(i=0;i< str_size;i++)
	{
		print_char8x8(ctx,((ctx->screen_txt_xsize - str_size)/2)+i,10-1,8,0);
	}


	// Upper left & right bar
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2)+(i-1),10-1,3,0);
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2),10-1,2,0);

	for(i=0;i< str_size;i++)
	{
		print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2)+i,80,' ',0);
	}

	// Upper bar
	for(i=0;i< str_size;i++)
	{
		print_char8x8(ctx,((ctx->screen_txt_xsize - str_size)/2)+i,10,' ',0);
	}

	// Middle left & right bar
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2)+(i-1),10,7,0);
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2),10,6,0);

	// Print the string
	print_str(ctx,temp_buffer,MAXTXTSIZE,((ctx->screen_txt_xsize-str_size)/2)+2,10,0,0);

	// Lower bar
	for(i=0;i<str_size;i++)
	{
		print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2)+i,10+1,9,0);
	}

	// Lower left & right bar
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2)+(i-1),10+1,5,0);
	print_char8x8(ctx,((ctx->screen_txt_xsize-str_size)/2),10+1,4,0);

	va_end( marker );

	return 0;
}

void init_display_buffer(ui_context * ctx)
{
	ctx->NUMBER_OF_ENTRIES_ON_DISPLAY = ctx->screen_txt_ysize - 2;          // (Display size minus top + bottom)
	ctx->NUMBER_OF_FILE_ON_DISPLAY = ctx->NUMBER_OF_ENTRIES_ON_DISPLAY - 1; // (Display size minus top + bottom + tittle)

	// Footprint : Current software / firmware version and title
	clear_line(ctx, ctx->screen_txt_ysize - 1, INVERTED);
	hxc_printf(ctx,LEFT_ALIGNED | INVERTED,0, ctx->screen_txt_ysize - 1,"FW %s",ctx->FIRMWAREVERSION);
	hxc_print(ctx,CENTER_ALIGNED | INVERTED,0,ctx->screen_txt_ysize - 1,(char*)title_msg);
	hxc_print(ctx,RIGHT_ALIGNED | INVERTED,0,ctx->screen_txt_ysize - 1,(char*)copyright_msg);

	// Top header : current folder path
	clear_line(ctx, 0, INVERTED);
	hxc_print(ctx,LEFT_ALIGNED|INVERTED,0,0, (char*)cur_folder_msg);

	// Startup message
	hxc_print(ctx,CENTER_ALIGNED,0,HELP_Y_POS+1, (char*)startup_msg);
}

char to_lower(char c)
{
	if(c>='A' && c<='Z')
	{
		return c + ('a' - 'A');
	}

	return c;
}

// https://stackoverflow.com/questions/27303062/strstr-function-like-that-ignores-upper-or-lower-case
char* stristr( const char* str1, const char* str2 )
{
    const char* p1 = str1 ;
    const char* p2 = str2 ;
    const char* r = *p2 == 0 ? str1 : 0 ;

    while( *p1 && *p2 )
    {
        if( to_lower( (unsigned char)*p1 ) == to_lower( (unsigned char)*p2 ) )
        {
            if( r == 0 )
            {
                r = p1 ;
            }

            p2++ ;
        }
        else
        {
            p2 = str2 ;
            if( r != 0 )
            {
                p1 = r + 1 ;
            }

            if( to_lower( (unsigned char)*p1 ) == to_lower( (unsigned char)*p2 ) )
            {
                r = p1 ;
                p2++ ;
            }
            else
            {
                r = 0 ;
            }
        }

        p1++ ;
    }

    return *p2 == 0 ? (char*)r : 0 ;
}

#ifdef DEBUG

void print_hex_buffer(ui_context * ctx,unsigned char * buffer, int size)
{
	int c,i;
	int x,y;

	c=0;

	x=0;
	y=0;
	for(i=0;i<size;i++)
	{
		x = ( (c & 0xF) * 3 );
		hxc_printf(ctx,LEFT_ALIGNED,x,y,"%.2X ", buffer[i]);
		c++;
		if( !( c & 0xF ) )
		{
			y++;
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
			hxc_printf(ctx,LEFT_ALIGNED,x,y,"%c", buffer[i]);
		}
		else
		{
			hxc_print(ctx,LEFT_ALIGNED,x,y,".");
		}
		c++;
		if(!(c&0xF))
		{
			y++;
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
