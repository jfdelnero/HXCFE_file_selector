/*
//
// Copyright (C) 2017-2018 Jean-François DEL NERO
//
// This file is part of the FontCP&P software
//
// FontCP&P may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// FontCP&P is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// FontCP&P is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FontCP&P; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include "bmp_file.h"

#define FONT_X_SIZE 8
#define FONT_Y_SIZE 8

#define FONT_X_SRCSTEP_SIZE 8
#define FONT_Y_SRCSTEP_SIZE 8

#define FONT_MATRIX_X_SIZE 16
#define FONT_MATRIX_Y_SIZE 16

typedef struct font_spec_
{
	unsigned int nb_of_chars;

	unsigned int char_x_size;
	unsigned int char_y_size;

	unsigned int x_char_step;
	unsigned int y_char_step;

	unsigned int chars_per_line;
	unsigned int chars_per_row;

	unsigned int x_offset;
	unsigned int y_offset;
}font_spec;

unsigned char framebuffer[(128*64) / 8];

void printbuf(unsigned char * buf,int size)
{
	int i;

	for(i=0;i<size;i++)
	{
		if(!(i&0xF))
		{
			if(i)
				printf("\n\t");
			else
				printf("\x7B\n\t");
		}
		printf("0x%.2X",buf[i]);
		if( i < size - 1 )
			printf(",");
		else
		{
			printf("\n\x7D\x3B\n");
		}
	}
	printf("\n");
}

int getpixstate(bitmap_data * bdata, int xpos, int ypos)
{
	if( (xpos < bdata->xsize) && (ypos < bdata->ysize) )
	{
		if( bdata->data[( bdata->xsize * ypos ) + xpos] )
		{
			return 1;
		}
	}
	else
	{
		printf("getpixstate : outside bmp ! : %d x %d\n", xpos, ypos );
	}
	return 0;
}

void build_framebuffer(bitmap_data * bdata)
{
	int x,y,bitoffset;
	int page,bit,seg;

	for(y=0;y<bdata->ysize;y++)
	{
		for(x=0;x<bdata->xsize;x++)
		{
			if(getpixstate(bdata, x, y))
			{
				page = y / 8;
				bit = y & 7;
				seg = x;
				bitoffset = (bdata->xsize * 8 * page) + (seg * 8) + bit;

				framebuffer[bitoffset/8] |= (0x01<< (bitoffset&7));
			}
		}
	}
}

void set_ssd1306_pix(unsigned char * buf,int x,int y,int xsize,int ysize)
{
	int bitoffset;
	int page,bit,seg;

	if( (x < xsize) && (y < ysize) )
	{
		page = y / 8;
		bit = y & 7;
		seg = x;

		bitoffset = (xsize * 8 * page) + (seg * 8) + bit;

		buf[bitoffset/8] |= (0x01<< (bitoffset&7));
	}
}

void set_char_pix(unsigned char * buf,int x,int y,int xsize,int ysize)
{
	int bitoffset;

	if( (x < xsize) && (y < ysize) )
	{
		bitoffset = (xsize * y) + x;

		buf[bitoffset/8] |= (0x80 >> (bitoffset&7));
	}
	else
	{
		printf("set_char_pix : outside bitmap area ! : %d x %d\n", x, y );
	}
}

unsigned char * extractchar(bitmap_data * bdata,int x,int y,int xsize,int ysize, int * bufsize)
{
	unsigned char * charbuf;
	int final_xsize,final_ysize;
	int i_x,i_y;

	final_xsize = xsize;

	if(ysize&7)
		final_ysize = ((ysize&~7) + 8);
	else
		final_ysize = ysize;

	*bufsize = ((final_xsize*final_ysize)/8);

	charbuf = malloc((final_xsize*final_ysize)/8);
	if(charbuf)
	{
		memset(charbuf,0x00,(final_xsize*final_ysize)/8);

		for(i_x = 0; i_x < xsize;i_x++)
		{
			for(i_y = 0; i_y < ysize;i_y++)
			{
				if(getpixstate(bdata, x + i_x, y + i_y))
				{
					//set_ssd1306_pix(charbuf,i_x,i_y+2,final_xsize,final_ysize);
					set_char_pix(charbuf,i_x,i_y,final_xsize,final_ysize);
				}
			}
		}
	}

	return charbuf;
}

int main(int argc, char *argv[])
{
	int bufsize,xchar,ychar,bufoffset;
	bitmap_data bdata;
	unsigned char * char_sprite;
	unsigned char * finalfontbuffer;
	font_spec ifnt;

	printf("FontCP&P v0.8\n");
	printf("(c) Jean-François DEL NERO / HxC2001 2017-2018\n\n");

	memset(&framebuffer,0,sizeof(framebuffer));

	if(argc > 1)
	{
		// Default font : 256 chars 8x8
		ifnt.nb_of_chars = 256;
		ifnt.char_x_size = FONT_X_SIZE;
		ifnt.char_y_size = FONT_Y_SIZE;
		ifnt.x_char_step = FONT_X_SRCSTEP_SIZE;
		ifnt.y_char_step = FONT_Y_SRCSTEP_SIZE;
		ifnt.x_offset = 0;
		ifnt.y_offset = 0;
		ifnt.chars_per_line = FONT_MATRIX_X_SIZE;
		ifnt.chars_per_row = FONT_MATRIX_Y_SIZE;

		if(!bmp_load(argv[1],&bdata))
		{

			printf("Bmp Loaded... Xsize: %d, Ysize: %d\n",bdata.xsize,bdata.ysize);

			// Get the sprite size.
			char_sprite = extractchar(&bdata,0,0,ifnt.char_x_size,ifnt.char_y_size,&bufsize);

			free(char_sprite);

			printf("Char sprite size : %d total table size : %d\n",bufsize,ifnt.nb_of_chars * bufsize);

			finalfontbuffer = malloc(ifnt.nb_of_chars * bufsize);
			if(finalfontbuffer)
			{
				memset(finalfontbuffer,0,ifnt.nb_of_chars * bufsize);
				bufoffset=0;

				for(ychar = 0; ychar < ifnt.chars_per_row; ychar++ )
				{
					for(xchar = 0; xchar < ifnt.chars_per_line ; xchar++)
					{
						char_sprite = extractchar(&bdata, xchar*ifnt.x_char_step, ychar*ifnt.y_char_step, ifnt.char_x_size, ifnt.char_y_size, &bufsize);

						printf("Char %d, size:%d, x:%d, y:%d, offset:%d\n", \
								(ychar * ifnt.chars_per_line) + xchar, \
								bufsize,
								xchar * ifnt.x_char_step,
								ychar * ifnt.y_char_step,
								bufoffset);

						memcpy( finalfontbuffer+bufoffset, char_sprite, bufsize);

						bufoffset += bufsize;

						free(char_sprite);
					}
				}
			}

			printf("\nconst unsigned char font[] =\n");
			printbuf( (unsigned char*)finalfontbuffer, ifnt.nb_of_chars * bufsize);

			free(finalfontbuffer);
		}
	}
}
