/*
//
// Copyright (C) 2017-2020 Jean-François DEL NERO / HxC2001
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

// default settings

#define FONT_X_SIZE 8
#define FONT_Y_SIZE 8

#define FONT_X_SRCSTEP_SIZE 8
#define FONT_Y_SRCSTEP_SIZE 8

#define FONT_MATRIX_X_SIZE 16
#define FONT_MATRIX_Y_SIZE 16

#define MAX_PARAM_ARG_SIZE 512

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

	unsigned int chars_y_align;

	unsigned int src_font_start_char;
}font_spec;

unsigned char framebuffer[(128*64) / 8];

void generate_source_files(char * outfile, unsigned char * buf, int size, font_spec * font)
{
	int i;
	int charsize;
	FILE * f;
	char filename[MAX_PARAM_ARG_SIZE];

	f = 0;

	if(strlen(outfile))
	{
		// Create header file
		strcpy(filename,outfile);
		strcat(filename,".h");
		f = fopen(filename , "w");
		if(f)
		{
			fprintf(f,"////////////////////////////////////\n");
			fprintf(f,"//  FontCP&P generated font data  //\n");
			fprintf(f,"////////////////////////////////////\n");
			fprintf(f,"\n");

			fprintf(f,"#ifndef FONT_HEADER\n");
			fprintf(f,"#define FONT_HEADER\n");
			fprintf(f,"typedef struct _font_type\n");
			fprintf(f,"{\n");
			fprintf(f,"\tunsigned int nb_of_chars;\n");
			fprintf(f,"\tunsigned int char_x_size;\n");
			fprintf(f,"\tunsigned int char_y_size;\n");
			fprintf(f,"\tunsigned int char_size;\n");
			fprintf(f,"\tunsigned int buffer_size;\n");
			fprintf(f,"\tconst unsigned char * font_data;\n");
			fprintf(f,"}font_type;\n");

			fprintf(f,"#endif\n");

			fprintf(f,"\n");
			fprintf(f,"extern const unsigned char data_%s[];\n",outfile);
			fprintf(f,"extern font_type %s;\n",outfile);

			fclose(f);
		}

		strcpy(filename,outfile);
		strcat(filename,".c");

		f = fopen(filename , "w");
	}

	if(!f)
		f = stdout;

	fprintf(f,"////////////////////////////////////\n");
	fprintf(f,"//  FontCP&P generated font data  //\n");
	fprintf(f,"////////////////////////////////////\n");
	fprintf(f,"\n");

	fprintf(f,"#include \x22%s.h\x22\n",outfile);

	fprintf(f,"\n");
	fprintf(f,"font_type %s=\n",outfile);
	fprintf(f,"{\n");
	fprintf(f,"\t%d,\n",font->nb_of_chars);
	fprintf(f,"\t%d,\n",font->char_x_size);
	fprintf(f,"\t%d,\n",font->char_y_size);

	charsize = font->char_x_size * font->char_y_size;
	if(charsize & 7)
		charsize = ((charsize & ~7) + 8);
	fprintf(f,"\t%d,\n",charsize / 8);
	fprintf(f,"\t%d,\n",size);
	fprintf(f,"\tdata_%s\n",outfile);
	fprintf(f,"};\n");

	fprintf(f,"\n");

	fprintf(f,"const unsigned char data_%s[] =\n",outfile);

	for(i=0;i<size;i++)
	{
		if(!(i&0xF))
		{
			if(i)
			{
				fprintf(f,"\n\t");
			}
			else
			{
				fprintf(f,"\x7B\n\t");
			}
		}

		fprintf(f,"0x%.2X",buf[i]);

		if( i < size - 1 )
		{
			fprintf(f,",");
		}
		else
		{
			fprintf(f,"\n\x7D\x3B\n");
		}
	}

	if( f != stdout )
	{
		fclose(f);
	}
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

unsigned char * extractchar(bitmap_data * bdata,int x,int y,int xsize,int ysize, int * bufsize,int align_y)
{
	unsigned char * charbuf;
	int final_xsize,final_ysize;
	int i_x,i_y;

	final_xsize = xsize;

	if(ysize & (align_y-1))
		final_ysize = ( ( ysize & ~(align_y-1) ) + align_y);
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

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,sizeof(option));
	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,sizeof(option));

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':')
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] && j < ( MAX_PARAM_ARG_SIZE - 1 ) )
							{
								argtoparam[j]=argv[param][i];
								i++;
								j++;
							}
							argtoparam[j]=0;
							return 1;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						return 1;
					}
				}
			}
		}
		param++;
	}

	return 0;
}

void printhelp(char* argv[])
{
	printf("Options:\n");
	printf("  -help \t\t\t\t: This help\n");
	printf("  -file:[filename]\t\t\t: Input bmp file\n");
	printf("  -fontname:[name]\t\t\t: Output font name\n");
	printf("  -nbchars:[nb of characters]\t\t: Total number of characters\n");
	printf("  -cxsize:[nb of pixels]\t\t: Character x size\n");
	printf("  -cysize:[nb of pixels]\t\t: Character y size\n");
	printf("  -cxstep:[nb of pixels]\t\t: Character x step\n");
	printf("  -cystep:[nb of pixels]\t\t: Character y step\n");
	printf("  -xoffset:[nb of pixels]\t\t: Initial x offset\n");
	printf("  -yoffset:[nb of pixels]\t\t: Initial y offset\n");
	printf("  -charsperline:[nb of characters]\t: Characters per line\n");
	printf("  -charsperrow:[nb of characters]\t: Characters per row\n");
	printf("  -srcstartchar:[character number]\t: First character index in the font\n");
	printf("  -nofontpatch\t\t\t\t: Don't apply the final patch\n");
	printf("\n");
}

void patch_font( font_spec * ifnt, unsigned char * finalfontbuffer)
{
	unsigned char * charbuf;
	int final_xsize,final_ysize;
	int i_x,i_y;
	int spritesize;
	int cur_char;

	final_xsize = ifnt->char_x_size;

	if(ifnt->char_y_size & (ifnt->chars_y_align-1))
		final_ysize = ( ( ifnt->char_y_size & ~(ifnt->chars_y_align-1) ) + ifnt->chars_y_align);
	else
		final_ysize = ifnt->char_y_size;

	spritesize = ((final_xsize*final_ysize)/8);

	charbuf = malloc(spritesize);
	if(charbuf)
	{
		// Char 0 : Blank
		memset(charbuf,0x00, spritesize);
		cur_char = 0;
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 1 : Full on
		// ***
		// ***
		// ***
		memset(charbuf,0xFF, spritesize);
		cur_char = 1;
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 2 :
		// ***
		// *
		// *
		memset(charbuf,0x00, spritesize);
		cur_char = 2;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == 0) || (i_y == 0) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}

		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 3 :
		// ***
		//   *
		//   *
		memset(charbuf,0x00, spritesize);
		cur_char = 3;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == (ifnt->char_x_size-1)) || (i_y == 0) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}

		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 4 :
		// *
		// *
		// ***
		memset(charbuf,0x00, spritesize);
		cur_char = 4;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == 0) || (i_y == (ifnt->char_y_size-1)) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}

		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 5 :
		//   *
		//   *
		// ***
		memset(charbuf,0x00, spritesize);
		cur_char = 5;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == (ifnt->char_x_size-1)) || (i_y == (ifnt->char_y_size-1)) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}

		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 6 :
		// *
		// *
		// *
		memset(charbuf,0x00, spritesize);
		cur_char = 6;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == 0) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 7 :
		//   *
		//   *
		//   *
		memset(charbuf,0x00, spritesize);
		cur_char = 7;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x == (ifnt->char_x_size-1)) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 8 :
		// ***
		//
		//
		memset(charbuf,0x00, spritesize);
		cur_char = 8;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_y == 0) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 9 :
		//
		//
		// ***
		memset(charbuf,0x00, spritesize);
		cur_char = 9;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_y == (ifnt->char_y_size-1)) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 10 : (Folder)
		//  *
		// ***
		//  *
		memset(charbuf,0x00, spritesize);
		cur_char = 10;

		for(i_x = 0; i_x < ifnt->char_x_size;i_x++)
		{
			for(i_y = 0; i_y < ifnt->char_y_size;i_y++)
			{
				if( (i_x != (ifnt->char_x_size-1)) && \
				    (i_y != (ifnt->char_y_size-1)) && \
				    (i_x != 0) && \
				    (i_y != 0) )
				{
					set_char_pix(charbuf, i_x, i_y, final_xsize, final_ysize);
				}
			}
		}
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		// Char 11 : (File)
		//
		//
		//
		memset(charbuf,0x00, spritesize);
		cur_char = 11;
		memcpy( finalfontbuffer+(cur_char*spritesize), charbuf, spritesize );

		free(charbuf);
	}

	return;
}

int main(int argc, char *argv[])
{

	int bufsize,xchar,ychar,bufoffset;
	bitmap_data bdata;
	unsigned char * char_sprite;
	unsigned char * finalfontbuffer;
	char filename[MAX_PARAM_ARG_SIZE];
	char fontname[MAX_PARAM_ARG_SIZE];
	char tmpparam[MAX_PARAM_ARG_SIZE];
	font_spec ifnt;

	printf("FontCP&P v0.8\n");
	printf("(c) Jean-François DEL NERO / HxC2001 2017-2019\n\n");

	memset(&framebuffer,0,sizeof(framebuffer));

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
	ifnt.chars_y_align = 1;
	ifnt.src_font_start_char = 0;

	// help option...
	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}

	memset(filename,0,sizeof(filename));
	if(isOption(argc,argv,"file",(char*)&filename)>0)
	{
		printf("Input file : %s\n",filename);
	}

	memset(fontname,0,sizeof(fontname));
	if(isOption(argc,argv,"fontname",(char*)&fontname)>0)
	{
		printf("Font name : %s\n",fontname);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"nbchars",(char*)&tmpparam)>0)
	{
		ifnt.nb_of_chars = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"cxsize",(char*)&tmpparam)>0)
	{
		ifnt.char_x_size = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"cysize",(char*)&tmpparam)>0)
	{
		ifnt.char_y_size = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"cxstep",(char*)&tmpparam)>0)
	{
		ifnt.x_char_step = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"cystep",(char*)&tmpparam)>0)
	{
		ifnt.y_char_step = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"xoffset",(char*)&tmpparam)>0)
	{
		ifnt.x_offset = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"yoffset",(char*)&tmpparam)>0)
	{
		ifnt.y_offset = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"charsperline",(char*)&tmpparam)>0)
	{
		ifnt.chars_per_line = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"charsperrow",(char*)&tmpparam)>0)
	{
		ifnt.chars_per_row = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"charsyalign",(char*)&tmpparam)>0)
	{
		ifnt.chars_y_align = atoi(tmpparam);
	}

	memset(tmpparam,0,sizeof(tmpparam));
	if(isOption(argc,argv,"srcstartchar",(char*)&tmpparam)>0)
	{
		ifnt.src_font_start_char = atoi(tmpparam);
	}

	if(strlen(filename))
	{
		if(!bmp_load(filename,&bdata))
		{
			printf("Total number of characters : %d characters\n",ifnt.nb_of_chars);
			printf("Character x size : %d pixels\n",ifnt.char_x_size);
			printf("Character y size : %d pixels\n",ifnt.char_y_size);
			printf("Character x step : %d pixels\n",ifnt.x_char_step);
			printf("Character y step : %d pixels\n",ifnt.y_char_step);
			printf("Initial x offset : %d pixel(s)\n",ifnt.x_offset);
			printf("Initial y offset : %d pixel(s)\n",ifnt.y_offset);
			printf("Characters per line : %d characters\n",ifnt.chars_per_line);
			printf("Characters per row : %d characters\n",ifnt.chars_per_row);
			printf("Character y size alignment : %d pixels\n",ifnt.chars_y_align);
			printf("Source font starting char : %d\n",ifnt.src_font_start_char);

			printf("\nBmp Loaded... Xsize: %d, Ysize: %d\n",bdata.xsize,bdata.ysize);

			// Get the sprite size.
			char_sprite = extractchar(&bdata,0,0,ifnt.char_x_size,ifnt.char_y_size,&bufsize,ifnt.chars_y_align);

			free(char_sprite);

			printf("Char sprite size : %d total table size : %d\n",bufsize,ifnt.nb_of_chars * bufsize);

			finalfontbuffer = malloc(ifnt.nb_of_chars * bufsize);
			if(finalfontbuffer)
			{
				memset(finalfontbuffer,0,ifnt.nb_of_chars * bufsize);
				bufoffset = ifnt.src_font_start_char * bufsize;

				for(ychar = 0; ychar < ifnt.chars_per_row; ychar++ )
				{
					for(xchar = 0; xchar < ifnt.chars_per_line ; xchar++)
					{
						char_sprite = extractchar(&bdata, xchar*ifnt.x_char_step, ychar*ifnt.y_char_step, ifnt.char_x_size, ifnt.char_y_size, &bufsize, ifnt.chars_y_align);

						printf("Char %d, size:%d, x:%d, y:%d, offset:%d\n", \
								(ychar * ifnt.chars_per_line) + xchar, \
								bufsize,
								xchar * ifnt.x_char_step,
								ychar * ifnt.y_char_step,
								bufoffset);

						if( bufoffset < ifnt.nb_of_chars * bufsize)
						{
							memcpy( finalfontbuffer+bufoffset, char_sprite, bufsize );
							free(char_sprite);
						}
						else
						{
							printf("Destination buffer full.\n");
							free(char_sprite);
							break;
						}

						bufoffset += bufsize;
					}
				}
			}

			if(isOption(argc,argv,"nofontpatch",(char*)&tmpparam) <= 0)
			{
				printf("Patching the font...\n");
				patch_font( &ifnt, finalfontbuffer);
			}

			generate_source_files(fontname, (unsigned char*)finalfontbuffer, ifnt.nb_of_chars * bufsize,&ifnt);

			free(finalfontbuffer);
		}
	}
	else
	{
		printhelp(argv);
	}
}
