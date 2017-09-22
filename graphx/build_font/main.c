#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bmp_file.h"

#define FONT_X_SIZE 8
#define FONT_Y_SIZE 8

#define FONT_X_SRCSTEP_SIZE 8
#define FONT_Y_SRCSTEP_SIZE 8

#define FONT_MATRIX_X_SIZE 16
#define FONT_MATRIX_Y_SIZE 16

unsigned char framebuffer[(128*64) / 8];

void printbuf(unsigned char * buf,int size)
{
	int i;
	printf("\n");
	for(i=0;i<size;i++)
	{
		if(!(i&0xF))
			printf("\n");

		printf("0x%.2X,",buf[i]);
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
		printf("outside bmp ! : %d x %d\n",xpos,ypos );
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

	page = y / 8;
	bit = y & 7;
	seg = x;

	bitoffset = (xsize * 8 * page) + (seg * 8) + bit;

	buf[bitoffset/8] |= (0x01<< (bitoffset&7));

}

void set_char_pix(unsigned char * buf,int x,int y,int xsize,int ysize)
{
	int bitoffset;

	bitoffset = (xsize * y) + x;

	buf[bitoffset/8] |= (0x80 >> (bitoffset&7));

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
	unsigned char * charbuf;
	unsigned char * finalfontbuffer;

	printf("build font\n");

	memset(&framebuffer,0,sizeof(framebuffer));

	if(argc > 1)
	{
		if(!bmp_load(argv[1],&bdata))
		{
			printf("Bmp Loaded... Xsize: %d, Ysize: %d\n",bdata.xsize,bdata.ysize);

			charbuf = extractchar(&bdata,0,0,FONT_X_SIZE,FONT_Y_SIZE,&bufsize);
			free(charbuf);
			printf("char size : %d total table size : %d\n",bufsize,256 * bufsize);

			finalfontbuffer = malloc(256 * bufsize);
			if(finalfontbuffer)
			{
				memset(finalfontbuffer,0,256 * bufsize);
				bufoffset=0;
				for(ychar = 0; ychar < FONT_MATRIX_Y_SIZE; ychar++)
				{
					for(xchar = 0; xchar < FONT_MATRIX_X_SIZE ; xchar++)
					{
						charbuf = extractchar(&bdata,xchar*FONT_X_SRCSTEP_SIZE,ychar*FONT_Y_SRCSTEP_SIZE,FONT_X_SIZE,FONT_Y_SIZE,&bufsize);
						printf("Char %d, size:%d, x:%d, y:%d, offset:%d\n",(ychar * FONT_MATRIX_X_SIZE)+xchar,bufsize,xchar*FONT_X_SRCSTEP_SIZE,ychar*FONT_Y_SRCSTEP_SIZE,bufoffset);
						memcpy( finalfontbuffer+bufoffset, charbuf, bufsize);
						bufoffset += bufsize;
						free(charbuf);
					}
				}
			}

			printbuf((unsigned char*)finalfontbuffer,256 * bufsize);

			free(finalfontbuffer);
		}
	}
}
