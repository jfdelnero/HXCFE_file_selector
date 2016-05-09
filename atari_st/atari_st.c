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

//#define DBGMODE 1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mint/osbind.h>
#include <time.h>
#include <vt52.h>


#ifdef __VBCC__
# include <tos.h>
#else
# include <mint/osbind.h>
# include <mint/linea.h>
# include "libc/snprintf/snprintf.h"
#endif

#include "conf.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "keymap.h"
#include "hardware.h"

static unsigned char floppydrive;
static unsigned char datacache[512*9];
static unsigned char valid_cache;
unsigned char g_color;
unsigned long old_physical_adr;

volatile unsigned short io_floppy_timeout;

unsigned char * screen_buffer;
unsigned char * screen_buffer_aligned;
unsigned char * screen_buffer_backup;
unsigned char * screen_buffer_backup_aligned;

unsigned short SCREEN_YRESOL;

static unsigned char CIABPRB_DSKSEL;

static unsigned char * mfmtobinLUT_L;
static unsigned char * mfmtobinLUT_H;

#define MFMTOBIN(W) ( mfmtobinLUT_H[W>>8] | mfmtobinLUT_L[W&0xFF] )

static unsigned short * track_buffer;
static unsigned short * track_buffer_wr;

static unsigned char validcache;

unsigned short sector_pos[16];

unsigned char keyup;

unsigned long timercnt;

#ifndef BMAPTYPEDEF
#define BMAPTYPEDEF

typedef  struct _bmaptype
{
   int type;
   int Xsize;
   int Ysize;
   int size;
   int csize;
   unsigned char * data;
}bmaptype __attribute__ ((aligned (2)));

#endif

void waitus(int centus)
{
}

  static unsigned long colortable[] = {
	0x002, 0xFFF, 0x0f0, 0x00f,
	0x000, 0xFFF, 0x0f0, 0x00f,
	0xFFF, 0x000, 0x0f0, 0x00f,
	0x030, 0xFFF, 0x0f0, 0x00f,
	0x300, 0xFFF, 0x0f0, 0x00f,
	0x303, 0xFFF, 0x0f0, 0x00f,
	0x999, 0x000, 0x0f0, 0x00f,
	0xFFF, 0x343, 0x0f0, 0x00f,
	0xF33, 0xFFF, 0x0f0, 0x00f,
	0xF0F, 0xFFF, 0x0f0, 0x00f,
	0xFFF, 0x0F0, 0x0f0, 0x00f,
	0xFF0, 0xFFF, 0x0f0, 0x00f,
	0x000, 0xF00, 0x0f0, 0x00f,
	0x000, 0x0F0, 0x0f0, 0x00f,
	0x000, 0x00F, 0x0f0, 0x00f,
	0x004, 0xFFF, 0x0f0, 0x00f
};

void waitms(int ms)
{
}

void testblink()
{
}

void alloc_error()
{
	hxc_printf_box(0,"ERROR: Memory Allocation Error -> No more free mem ?");
	for(;;);
}

/********************************************************************************
*                              FDC I/O
*********************************************************************************/
int jumptotrack(unsigned char t)
{
	unsigned short i,j;
	unsigned char data[512];

	Floprd( &data, 0, floppydrive, 1, t, 0, 1 );

	return 1;
};

int test_drive(int drive)
{
	return 0;
}

int waitindex()
{
	return 0;
}

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	int ret,retry;

	valid_cache =0;
	retry=3;

	ret=1;
	while(retry && ret)
	{
		ret=Flopwr( data, 0, floppydrive, sectornum, 255, 0, 1 );
		retry--;
	}

	if(!ret)
		return 1;
	else
		return 0;
}

unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret,retry;

	retry=3;
	ret=0;
	if(!valid_cache || invalidate_cache)
	{
		if(sectornum<10)
		{
			ret=1;
			while(retry && ret)
			{
				ret=Floprd( datacache, 0, floppydrive, 0, 255, 0, 9 );
				retry--;
			}

			memcpy((void*)data,&datacache[sectornum*512],512);
			valid_cache=0xFF;
		}
	}
	else
	{
		memcpy((void*)data,&datacache[sectornum*512],512);
	}

	if(!ret)
		return 1;
	else
		return 0;
}

void init_fdc(unsigned char drive)
{
	unsigned short i,ret;

	valid_cache = 0;
	floppydrive = drive;
	Floprate( floppydrive, 2);

	ret = Floprd( &datacache, 0, floppydrive, 0, 255, 0, 1 );
}

/********************************************************************************
*                          Joystick / Keyboard I/O
*********************************************************************************/

unsigned char Joystick()
{
	return 0;
}


unsigned char Keyboard()
{
	return Cconin()>>16;
}

int kbhit()
{
	return 0;
}

void flush_char()
{
}

void wait_released_key()
{
	while(!(Keyboard()&0x80));
}

unsigned char get_char()
{
	unsigned char buffer;
	unsigned char key,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;
	while(!(Keyboard()&0x80));

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				if(key&0x80)
				{
					c=1;
				}
			}while(key&0x80);
			waitms(55);
			c--;

		}while(c);

		i=0;
		do
		{
			function_code=char_keysmap[i].function_code;
			key_code=char_keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}


unsigned char wait_function_key()
{
	unsigned char key,joy,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;

	if( keyup == 1 )
	{
		waitms(250);
	}

	do
	{
		c=1;
		do
		{
			do
			{
				key=Keyboard();
				joy=Joystick();
				if(key&0x80 && !joy)
				{
					c=1;

					keyup = 2;
				}
			}while(key&0x80 && !joy);

			waitms(55);

			c--;

		}while(c);

		if(keyup)
			keyup--;

		if(joy)
		{
			if(joy&0x10)
			{
				while(Joystick()&0x10);

				return FCT_SELECT_FILE_DRIVEA;
			}
			if(joy&2)
				return FCT_DOWN_KEY;
			if(joy&1)
				return FCT_UP_KEY;
			if(joy&4)
				return FCT_RIGHT_KEY;
			if(joy&8)
				return FCT_LEFT_KEY;
		}

		i=0;
		do
		{
			function_code=keysmap[i].function_code;
			key_code=keysmap[i].keyboard_code;
			i++;
		}while((key_code!=key) && (function_code!=FCT_NO_FUNCTION) );

	}while(function_code==FCT_NO_FUNCTION);

	return function_code;
}

/********************************************************************************
*                              Display Output
*********************************************************************************/

void setvideomode(int mode)
{

}

void initpal()
{
	volatile unsigned short * ptr;

	ptr=(unsigned short *)0xFF8240;
	*ptr=colortable[((g_color&0xF)*4)+0];
	ptr=(unsigned short *)0xFF8242;
	*ptr=colortable[((g_color&0xF)*4)+2];
	ptr=(unsigned short *)0xFF8244;
	*ptr=colortable[((g_color&0xF)*4)+3];
	ptr=(unsigned short *)0xFF8246;
	*ptr=colortable[((g_color&0xF)*4)+1];
}

void set_color_scheme(unsigned char color)
{
	g_color = color;
    Supexec(initpal);
}

int init_display()
{
	unsigned short loop,yr;
	unsigned long k,i;

	screen_buffer_backup_aligned=(unsigned char*)malloc(16*1024 + ((32*1024) + 256) + ((8*1000) + 256));
//	memset(screen_buffer_backup_aligned,0,16*1024 + ((32*1024) + 256) + ((8*1000) + 256));

	SCREEN_YRESOL=200;


	old_physical_adr=(unsigned long)Physbase();

	screen_buffer=(unsigned char*) (screen_buffer_backup_aligned + 16*1024) ;


	screen_buffer_aligned = (unsigned char*)(((unsigned long)screen_buffer| 0xff)+1);

	screen_buffer_backup=(unsigned char*)screen_buffer_backup_aligned + ( 16*1024 + ((32*1024) + 256) );
	screen_buffer_backup_aligned = (unsigned char*)(((unsigned long)screen_buffer_backup| 0xff)+1);

	Blitmode(1);

    Setscreen( -1, screen_buffer_aligned, 1 );
	g_color=0;
    Supexec(initpal);

	yr= get_vid_mode();
	if(yr>290)
	{
		SCREEN_YRESOL=256;
	}
	else
	{
		SCREEN_YRESOL=200;
	}

	// Number of free line to display the file list.

	disablemousepointer();

	return 0;
}

unsigned short get_vid_mode()
{
	return 0;
}

void disablemousepointer()
{

}

void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
	unsigned short j,k,l,c1;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;

	ptr_dst = (unsigned char*)membuffer;
	ptr_src = (unsigned char*)&font->data[0];

	x = x>>3;
	x = ((x&(~0x1))<<1)+(x&1);//  0 1   2 3
	l = (y*160) + (x);
	k = ((c>>4)*(8*8*2))+(c&0xF);

	for(j=0;j<8;j++)
	{
		ptr_dst[l]   = ptr_src[k];
		ptr_dst[l+2] = ptr_src[k];
		k = k + (16);
		l = l + (160);
	}
}

void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y)
{
	unsigned short i,j,k,l,x_offset,base_offset;
	unsigned short *ptr_src;
	unsigned short *ptr_dst;

	ptr_dst=(unsigned short*)membuffer;
	ptr_src=(unsigned short*)&sprite->data[0];

	k=0;
	l=0;
	base_offset=((y*160)+ (((x>>2)&(~0x3))))/2;
	for(j=0;j<(sprite->Ysize);j++)
	{
		l=base_offset +(80*j);
		for(i=0;i<(sprite->Xsize/16);i++)
		{
			ptr_dst[l]=ptr_src[k];
			l++;
			ptr_dst[l]=ptr_src[k];
			l++;
			k++;
		}
	}
}

void h_line(unsigned short y_pos,unsigned short val)
{
	unsigned short *ptr_dst;
	unsigned short i,ptroffset;

	ptr_dst = (unsigned short*)screen_buffer_aligned;
	ptroffset = 80 * y_pos;

	for(i=0;i<80;i++)
	{
		ptr_dst[ptroffset+i] = val;
	}
}

void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill)
{
	unsigned short *ptr_dst;
	unsigned short i,j,ptroffset,x_size;

	ptr_dst = (unsigned short*)screen_buffer_aligned;

	x_size = ((x_p2-x_p1)/16)*2;

	for(j=0;j<(y_p2-y_p1);j++)
	{
		for(i=0;i<x_size;i++)
		{
			ptr_dst[ptroffset+i] = fillval;
		}
		ptroffset = 80* (y_p1+j);
	}
}

void invert_line(unsigned short x_pos,unsigned short y_pos)
{
	unsigned char i,j;
	unsigned short *ptr_dst;
	unsigned short ptroffset;

	for(j=0;j<8;j++)
	{
		ptr_dst = (unsigned short*)screen_buffer_aligned;
		ptroffset = 80* (y_pos+j);

		for(i=0;i<80;i++)
		{
			ptr_dst[ptroffset+i] = ptr_dst[ptroffset+i]^0xFFFF;
		}
	}
}

void restore_box()
{
	memcpy(&screen_buffer_aligned[160*70],screen_buffer_backup_aligned, (8*1000) + 256);
}

void su_reboot()
{
	asm("move.l #4,A6");
	asm("move.l (A6),A0");
	asm("move.l A0,-(SP)");
	asm("rts");
}

void reboot()
{
	Supexec(su_reboot);
}

void ithandler(void)
{
	timercnt++;

	io_floppy_timeout++;

	if( ( Keyboard() & 0x80 )  && !Joystick())
	{
		keyup  = 2;
	}
}

void init_timer()
{
}

void sleep(int secs)
{

}
