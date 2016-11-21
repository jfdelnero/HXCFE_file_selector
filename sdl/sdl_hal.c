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
#include <malloc.h>
#include <string.h>

#include "conf.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "keymap.h"

#include "../hxcfeda.h"

#include <SDL/SDL.h>

#define VIRT_VERSIONCODE "v3.0.8.5a"

#define DEPTH    2 /* 1 BitPlanes should be used, gives eight colours. */
#define COLOURS  2 /* 2^1 = 2                                          */

#define BLACK 0x002           /*  RGB values for the four colors used.   */
#define RED   0xFFF
#define GREEN 0x0f0
#define BLUE  0x00f

// SDL Stuff
SDL_Surface	*screen;
SDL_Surface	*bBuffer;
SDL_Rect	rScreen;
SDL_Rect	rBuffer;
SDL_Color   *colors;

unsigned char * screen_buffer;
unsigned char * screen_buffer_backup;
unsigned short SCREEN_XRESOL;
unsigned short SCREEN_YRESOL;


static unsigned char validcache;

unsigned short sector_pos[16];

unsigned char keyup;

unsigned long timercnt;

int track_number,number_of_sector;
unsigned long virt_lba;

#ifndef BMAPTYPEDEF
#define BMAPTYPEDEF

direct_access_status_sector virtual_hxcfe_status;


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

void alloc_error()
{
	hxc_printf_box(0,"ERROR: Memory Allocation Error -> No more free mem ?");
	for(;;);
}

int jumptotrack(unsigned char t)
{
	track_number = t;
}

int test_drive(int drive)
{
	if(drive)
		return 0;
	else
		return 1;
}

int write_mass_storage(unsigned long lba, unsigned char * data)
{
	FILE *f;

	f = fopen("/dev/sdc1","wb");
	if(f)
	{
		fseek(f,lba*512,SEEK_SET);
		fwrite(data,512,1,f);
		fclose(f);
	}

	return 0;
}

int read_mass_storage(unsigned long lba, unsigned char * data)
{
	FILE *f;

	f = fopen("/dev/sdc1","rb");
	if(f)
	{
		fseek(f,lba*512,SEEK_SET);
		fread(data,512,1,f);
		fclose(f);
	}
	return 0;
}

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	direct_access_cmd_sector  * da_cmd;

	if(track_number!=255)
		return 0;

	if(!sectornum)
	{
		da_cmd = (direct_access_cmd_sector  *)data;
		switch(da_cmd->cmd_code)
		{
			case 0:
				virtual_hxcfe_status.last_cmd_status=0;
				virtual_hxcfe_status.cmd_cnt++;
				virtual_hxcfe_status.sd_status=0x00;
			break;

			case 1:
				virtual_hxcfe_status.lba_base = ( ((unsigned long)da_cmd->parameter_3<<24) |
				                                  ((unsigned long)da_cmd->parameter_2<<16) |
												  ((unsigned long)da_cmd->parameter_1<< 8) |
												  ((unsigned long)da_cmd->parameter_0<< 0) );

				if((da_cmd->parameter_4==0xA5) || (da_cmd->parameter_4==0x5A))
				{
					virtual_hxcfe_status.write_locked=0x00;
				}

				number_of_sector = da_cmd->parameter_5;
				if((number_of_sector==0) || (number_of_sector>64))
					number_of_sector=8;
				virtual_hxcfe_status.last_cmd_status=0;
				virtual_hxcfe_status.cmd_cnt++;
			break;
			case 2:
				track_number = da_cmd->parameter_0;
			break;
			default:
				virtual_hxcfe_status.last_cmd_status=1;
			break;
		}
	}
	else
	{
		if(sectornum > number_of_sector)
		{
			return 0;
		}

		write_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), data);
	}

	return 1;
}


unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	if(track_number!=255)
		return 0;

	if(!sectornum)
	{
		memset(data,0,512);
		memcpy(data,&virtual_hxcfe_status,sizeof(virtual_hxcfe_status));
	}
	else
	{
		if(sectornum > number_of_sector)
		{
			return 0;
		}

		read_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), data);
	}
	return 1;
}

void init_fdc(unsigned char drive)
{
	memset(&virtual_hxcfe_status,0,sizeof(virtual_hxcfe_status));
	memcpy((char*)&virtual_hxcfe_status.DAHEADERSIGNATURE,	(const char *)"HxCFEDA",	strlen("HxCFEDA"));
	memcpy((char*)&virtual_hxcfe_status.FIRMWAREVERSION,	(const char *)VIRT_VERSIONCODE,	strlen(VIRT_VERSIONCODE));
	virt_lba = 0;
	number_of_sector = 0;
	jumptotrack(255);
}

/********************************************************************************
*                          Joystick / Keyboard I/O
*********************************************************************************/

unsigned char Joystick()
{
	return 0x00;
}


unsigned char Keyboard()
{
	unsigned char key;
	SDL_Event event;
	SDL_KeyboardEvent *key_event;
	
	SDL_GetKeyState(NULL);
	key = 0;
	if(SDL_PollEvent(&event))
	{
		if( event.type == SDL_KEYDOWN)
		{
			key = 0x80;
			key_event = &event.key;
			printf("key: %.4x\n",key_event->keysym.scancode);
			key |= key_event->keysym.scancode;
		}
	}
	
	return key;
}

void flush_char()
{
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

int update_screen()
{
	unsigned char *buffer_dat;
	int i;

	buffer_dat = (unsigned char *)bBuffer->pixels;

	SDL_LockSurface(bBuffer);
	memcpy(buffer_dat,screen_buffer,(SCREEN_XRESOL*SCREEN_YRESOL));
	SDL_UnlockSurface(bBuffer);

	SDL_BlitSurface( bBuffer, NULL, screen, &rBuffer );
	SDL_UpdateRect( screen, 0, 0, 0, 0 );

}

int init_display()
{
	unsigned short loop,yr;
	int i;

	track_number = 0;

	SCREEN_XRESOL = 640;
	SCREEN_YRESOL = 240;

	screen_buffer = malloc(SCREEN_XRESOL*SCREEN_YRESOL);
	screen_buffer_backup=(unsigned char*)malloc(8*1024*2);

	SDL_Init( SDL_INIT_VIDEO );

	screen = SDL_SetVideoMode( SCREEN_XRESOL, SCREEN_YRESOL, 8, SDL_SWSURFACE);

	bBuffer = SDL_CreateRGBSurface( SDL_HWSURFACE, screen->w,
					screen->h,
					screen->format->BitsPerPixel,
					screen->format->Rmask,
					screen->format->Gmask,
					screen->format->Bmask,
					screen->format->Amask);

	colors=(SDL_Color *)malloc(sizeof(SDL_Color)*256);

	rBuffer.x = 0;
	rBuffer.y = 0;
	rBuffer.w = bBuffer->w;
	rBuffer.h = bBuffer->h;

// copie  de la palette
	for(i=0;i<256;i++)
	{
		colors[i].r = i;
		colors[i].g = i;
		colors[i].b = i;
	}
	SDL_SetColors(bBuffer, colors, 0, 256);

	update_screen();

	return 0;
}

void disablemousepointer()
{
}

void initpal()
{

}

unsigned char set_color_scheme(unsigned char color)
{
	return color;
}

void print_char8x8(unsigned char * membuffer, bmaptype * font,unsigned short x, unsigned short y,unsigned char c)
{
	unsigned short i,j,k,l,c1;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;

	ptr_dst=(unsigned char*)membuffer;
	ptr_src=(unsigned char*)&font->data[0];

	ptr_dst=ptr_dst + ((y*SCREEN_XRESOL)+ x);
	ptr_src=ptr_src + (((c>>4)*(8*8*2))+(c&0xF));
	for(j=0;j<8;j++)
	{
		for(i=0;i<8;i++)
		{
			if(*ptr_src & (0x80>>i) )
				ptr_dst[i]= 0xFF;
			else
				ptr_dst[i]= 0x00;
		}
		ptr_src=ptr_src+16;
		ptr_dst=ptr_dst + SCREEN_XRESOL;
	}

	update_screen();

}

void display_sprite(unsigned char * membuffer, bmaptype * sprite,unsigned short x, unsigned short y)
{
	unsigned long i,j,k,l,x_offset,base_offset;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;

	ptr_dst=(unsigned char*)membuffer;
	ptr_src=(unsigned char*)&sprite->data[0];

	k=0;
	l=0;
	base_offset=(( y * SCREEN_XRESOL)+ x);
	for(j=0;j<(sprite->Ysize);j++)
	{
		l = base_offset + (SCREEN_XRESOL * j);
		k = (j * sprite->Xsize) / 8;
		for(i=0;i<(sprite->Xsize);i++)
		{
			if(ptr_src[k + (i>>3)] & (0x80>>i) )
				ptr_dst[l + i]= 0xFF;
			else
				ptr_dst[l + i]= 0x00;
		}
	}

	update_screen();
}

void h_line(unsigned short y_pos,unsigned short val)
{
	unsigned char *ptr_dst;
	unsigned long i,ptroffset;

	ptr_dst = screen_buffer;
	ptroffset = SCREEN_XRESOL * y_pos;

	for(i=0;i< (SCREEN_XRESOL/2);i++)
	{
		ptr_dst[ptroffset + (i*2)]=(val>>8)&0xFF;
		ptr_dst[ptroffset + (i*2) + 1]=(val)&0xFF;
	}

	update_screen();

}

void box(unsigned short x_p1,unsigned short y_p1,unsigned short x_p2,unsigned short y_p2,unsigned short fillval,unsigned char fill)
{
	unsigned short *ptr_dst;
	unsigned short i,j,ptroffset,x_size;

	ptr_dst=(unsigned short*)screen_buffer;

	x_size=(x_p2-x_p1);

	ptroffset = SCREEN_XRESOL * y_p1;
	for(j=0;j<(y_p2-y_p1);j++)
	{
		for(i=0;i<x_size/2;i++)
		{
			ptr_dst[ptroffset+(i*2)]=(fillval>>8);
			ptr_dst[ptroffset+(i*2)+1]=(fillval)&0xFF;
		}
		ptroffset = SCREEN_XRESOL * (y_p1+j);
	}

	update_screen();

}

void invert_line(unsigned short x_pos,unsigned short y_pos)
{
	unsigned short i,j;
	unsigned char *ptr_dst;
	unsigned long ptroffset;

	for(j=0;j<8;j++)
	{
		ptr_dst=(unsigned char*)screen_buffer;
		ptroffset=(SCREEN_XRESOL* (y_pos + j))+x_pos;

		for(i=0;i<SCREEN_XRESOL;i++)
		{
			ptr_dst[ptroffset+i]=ptr_dst[ptroffset+i]^0xFF;
		}
	}

	update_screen();

}

void save_box()
{
	//memcpy(screen_buffer_backup,&screen_buffer[160*70], 8*1024);
}

void restore_box()
{
//	memcpy(&screen_buffer[160*70],screen_buffer_backup, 8*1024);
	update_screen();

}

void reboot()
{
	exit(0);
	for(;;);
}

void ithandler(void)
{
	timercnt++;

	if( ( Keyboard() & 0x80 )  && !Joystick())
	{
		keyup  = 2;
	}
}

void init_timer()
{
}

char *strlwr(char *s)
{
	unsigned char *s1;

	s1=(unsigned char *)s;
	while(*s1) {
	if (isupper(*s1))
		*s1+='a'-'A';
	++s1;
	}
	return s;
}
