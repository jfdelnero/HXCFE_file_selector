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
#include <string.h>

#include "conf.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "hardware.h"


volatile unsigned short io_floppy_timeout;

unsigned char * screen_buffer_aligned;
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

int readtrack(unsigned short * track,unsigned short size,unsigned char waiti)
{
	return 1;
}

int writetrack(unsigned short * track,unsigned short size,unsigned char waiti)
{
	return 1;
}

// Fast Bin to MFM converter
int BuildCylinder(unsigned char * mfm_buffer,int mfm_size,unsigned char * track_data,int track_size,unsigned short lastbit,unsigned short * retlastbit)
{
	return 0;
}

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	return 1;
}


unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	return 1;
}

void init_fdc(unsigned char drive)
{
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
	return 0;
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

int init_display()
{
	unsigned short loop,yr;

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

void initpal()
{
}

void set_color_scheme(unsigned char color)
{
}

void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
}

void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y)
{
}

void h_line(unsigned short y_pos,unsigned short val)
{
}

void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill)
{
}

void invert_line(unsigned short x_pos,unsigned short y_pos)
{
}

void restore_box()
{
	memcpy(&screen_buffer_aligned[160*70],screen_buffer_backup_aligned, (8*1000) + 256);
}

void reboot()
{
//	_reboot();
	for(;;);
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
