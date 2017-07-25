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

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <SDL/SDL.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "conf.h"

#include "keysfunc_defs.h"
#include "keys_defs.h"
#include "keymap.h"

#include "graphx/bmaptype.h"

#include "color_table.h"

#include "gui_utils.h"

#include "slot_list_gen.h"

#include "../hxcfeda.h"


#define VIRT_VERSIONCODE "v3.X.X.Xa"

#define DEPTH    2 /* 1 BitPlanes should be used, gives eight colours. */
#define COLOURS  2 /* 2^1 = 2                                          */

#define BLACK 0x002           /*  RGB values for the four colors used.   */
#define RED   0xFFF
#define GREEN 0x0f0
#define BLUE  0x00f

#ifdef WIN32
HANDLE  hMassStorage;
#else
FILE   *hMassStorage;
#endif

// SDL Stuff
SDL_Surface	*screen;
SDL_Surface	*bBuffer;
SDL_Rect	rScreen;
SDL_Rect	rBuffer;
SDL_Color   *colors;

SDL_TimerID sdl_timer_id;

unsigned char * screen_buffer;
unsigned char * screen_buffer_backup;
uint16_t SCREEN_XRESOL;
uint16_t SCREEN_YRESOL;

uint16_t sector_pos[16];

unsigned char keyup;

uint32_t timercnt;

int track_number,number_of_sector;
uint32_t virt_lba;

unsigned char * keys_stat;
unsigned char last_key;

direct_access_status_sector virtual_hxcfe_status;

char dev_path[512];

static unsigned char datacache[512*16];
static unsigned char valid_cache;

void waitus(int centus)
{

}

void waitms(int ms)
{
	SDL_Delay(ms);
}

#ifdef WIN32
void sleep(int secs)
{
	int i;

	for(i=0;i<secs;i++)
	{
		waitms(1000);
	}
}
#endif

void alloc_error()
{
	hxc_printf_box(0,"ERROR: Memory Allocation Error -> No more free mem ?");
	for(;;);
}

int jumptotrack(unsigned char t)
{
	track_number = t;
	return 1;
}

int get_start_unit(char * path)
{
	return 0;
}

void close_disk_access()
{
#ifdef WIN32
	if(hMassStorage != INVALID_HANDLE_VALUE)
	{
		CloseHandle (hMassStorage);
		hMassStorage = INVALID_HANDLE_VALUE;
	}
#else
	if(hMassStorage)
	{
		fclose (hMassStorage);
		hMassStorage = 0;
	}
#endif
}

int write_mass_storage(unsigned long lba, unsigned char * data)
{
	#ifdef WIN32
	int locked;
	DWORD dwNotUsed;
	DWORD lDistLow,lDistHigh;

	#define FSCTL_LOCK_VOLUME            0x00090018
	#define FSCTL_UNLOCK_VOLUME          0x0009001C

	locked = 0;

	if (hMassStorage != INVALID_HANDLE_VALUE)
	{
		if(DeviceIoControl(hMassStorage,(DWORD) FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwNotUsed, NULL))
			locked = 1;

		// * 512 (64bits)
		lDistLow = lba << 1;
		lDistHigh = (lDistLow >> 24) & 0xFF;
		lDistLow <<= 8;
		if(lba & 0x80000000)
			lDistHigh += 0x00000100;

		SetFilePointer (hMassStorage, lDistLow, &lDistHigh, FILE_BEGIN);
		if (WriteFile (hMassStorage, data, 512, &dwNotUsed, NULL))
		{
			if(locked)
				DeviceIoControl(hMassStorage,(DWORD) FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwNotUsed, NULL);

			return 1;
		}

		if(locked)
			DeviceIoControl(hMassStorage,(DWORD) FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwNotUsed, NULL);
	}

	#else
	if(hMassStorage)
	{
		fseeko(hMassStorage,(off_t)lba*(off_t)512,SEEK_SET);
		fwrite(data,512,1,hMassStorage);
		return 1;
	}
	#endif

	return 0;
}

int read_mass_storage(unsigned long lba, unsigned char * data, int nbsector)
{
	#ifdef WIN32
	DWORD dwNotUsed;
	DWORD lDistLow,lDistHigh;

	if (hMassStorage != INVALID_HANDLE_VALUE)
	{
		// * 512 (64bits)
		lDistLow = lba << 1;
		lDistHigh = (lDistLow >> 24) & 0xFF;
		lDistLow <<= 8;
		if(lba & 0x80000000)
			lDistHigh += 0x00000100;

		SetFilePointer (hMassStorage, lDistLow, &lDistHigh, FILE_BEGIN);
		if (ReadFile (hMassStorage, data, nbsector*512, &dwNotUsed, NULL))
		{
			return 1;
		}
	}

	#else
	int ret;

	if(hMassStorage)
	{
		fseeko(hMassStorage,(off_t)lba*(off_t)512,SEEK_SET);
		ret = fread(data,nbsector*512,1,hMassStorage);
		return ret;
	}
	#endif

	return 0;
}

unsigned char writesector(unsigned char sectornum,unsigned char * data)
{
	direct_access_cmd_sector  * da_cmd;

	valid_cache=0;

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

		return write_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), data);
	}

	return 1;
}


unsigned char readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret;

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

		ret = 1;

		if(!valid_cache || invalidate_cache)
		{
			ret = read_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), datacache, number_of_sector);
			if(ret)
				valid_cache=0xFF;
		}

		if(ret)
			memcpy((void*)data,&datacache[(sectornum-1)*512],512);

		return ret;
	}
	return 1;
}

void init_fdc(int drive)
{
	#ifdef WIN32
	char drv_path[64];

	strcpy(drv_path,"\\\\.\\");
	strncat(drv_path,dev_path,sizeof(drv_path)-1);

	hMassStorage = CreateFile (drv_path, GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
					NULL);
	#else
	hMassStorage = fopen(dev_path,"rb+");
	#endif

	memset(&virtual_hxcfe_status,0,sizeof(virtual_hxcfe_status));
	memcpy((char*)&virtual_hxcfe_status.DAHEADERSIGNATURE,	(const char *)"HxCFEDA",	strlen("HxCFEDA"));
	memcpy((char*)&virtual_hxcfe_status.FIRMWAREVERSION,	(const char *)VIRT_VERSIONCODE,	strlen(VIRT_VERSIONCODE));
	virt_lba = 0;
	valid_cache = 0;
	number_of_sector = 9;
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
	int i;

	SDL_PumpEvents();

	i =0;
	while(keysmap[i].function_code)
	{
		if(keys_stat[sdl_scancode[keysmap[i].keyboard_code]])
		{
			last_key = keysmap[i].keyboard_code;
			//printf("k:%x f:%x\n",keysmap[i].keyboard_code,keysmap[i].function_code);
			return last_key;
		}
		i++;
	}

	return last_key | 0x80;
}

void flush_char()
{
}

unsigned char get_char()
{
	unsigned char key,i,c;
	unsigned char function_code,key_code;

	function_code=FCT_NO_FUNCTION;
	while(!(Keyboard()&0x80))
	{
		waitms(1);
	}

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
					waitms(1);
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
					waitms(1);
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
				while(Joystick()&0x10)
				{
					waitms(1);
				}

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

	buffer_dat = (unsigned char *)bBuffer->pixels;

	SDL_LockSurface(bBuffer);
	memcpy(buffer_dat,screen_buffer,(SCREEN_XRESOL*SCREEN_YRESOL));
	SDL_UnlockSurface(bBuffer);

	SDL_BlitSurface( bBuffer, NULL, screen, &rBuffer );
	SDL_UpdateRect( screen, 0, 0, 0, 0 );

	return 0;
}

uint32_t sdl_timer(Uint32 interval, void *param)
{
	SDL_Event evt;

	if(SDL_PollEvent(&evt))
	{
		switch(evt.type)
		{
			case SDL_QUIT:
				close_disk_access();
				SDL_Quit();

				exit(0);
			break;
		}
	}

	update_screen();

	return interval;
}

void init_timer()
{
	sdl_timer_id = SDL_AddTimer(30, sdl_timer, "a");
	if(!sdl_timer_id)
	{
		close_disk_access();
		SDL_Quit();
		exit(-1);
	}
}

int init_display()
{
	int i;

	track_number = 0;

	SCREEN_XRESOL = 640;
	SCREEN_YRESOL = 256;

	screen_buffer = malloc(SCREEN_XRESOL*SCREEN_YRESOL);
	if(!screen_buffer)
		return 1;

	memset(screen_buffer,0,SCREEN_XRESOL*SCREEN_YRESOL);

	screen_buffer_backup=(unsigned char*)malloc(8*1024*2);

	sdl_timer_id = 0;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER );

	screen = SDL_SetVideoMode( SCREEN_XRESOL, SCREEN_YRESOL, 8, SDL_SWSURFACE);

	last_key = 0x00;
	keys_stat = SDL_GetKeyState(NULL);

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

	// Default Palette init.
	for(i=0;i<256;i++)
	{
		colors[i].r = i;
		colors[i].g = i;
		colors[i].b = i;
	}
	SDL_SetColors(bBuffer, colors, 0, 256);

	update_screen();

	init_timer();
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
	int r,g,b,i;

	for(i=0;i<256;i++)
	{
		colors[i].r = i;
		colors[i].g = i;
		colors[i].b = i;
	}

	for(i=0;i<4;i++)
	{
		r = (colortable[((color&0x1F)*4) + i] & 0xF00)>>8;
		g = (colortable[((color&0x1F)*4) + i] & 0x0F0)>>4;
		b = (colortable[((color&0x1F)*4) + i] & 0x00F)>>0;

		switch(i)
		{
			case 0:
				colors[0].r = r * 16;
				colors[0].g = g * 16;
				colors[0].b = b * 16;
			break;
			case 1:
				colors[255].r = r * 16;
				colors[255].g = g * 16;
				colors[255].b = b * 16;
			break;
			case 2:
				colors[2].r = r * 16;
				colors[2].g = g * 16;
				colors[2].b = b * 16;
			break;
			case 3:
				colors[3].r = r * 16;
				colors[3].g = g * 16;
				colors[3].b = b * 16;
			break;
		}
	}
	SDL_SetColors(bBuffer, colors, 0, 256);

	return color;
}

void print_char8x8(unsigned char * membuffer, bmaptype * font,int x, int y,unsigned char c)
{
	int i,j;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;

	ptr_dst = (unsigned char*)membuffer;
	ptr_src = (unsigned char*)&font->data[0];

	if( y < SCREEN_YRESOL && x < SCREEN_XRESOL)
	{
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
	}

}

void display_sprite(unsigned char * membuffer, bmaptype * sprite,int x, int y)
{
	int i,j,k,l,base_offset;
	unsigned char *ptr_src;
	unsigned char *ptr_dst;

	ptr_dst = (unsigned char*)membuffer;
	ptr_src = (unsigned char*)&sprite->data[0];

	if( y < SCREEN_YRESOL && x < SCREEN_XRESOL)
	{
		k=0;
		l=0;
		base_offset=(( y * SCREEN_XRESOL)+ x);
		for(j=0;j<(sprite->Ysize);j++)
		{
			l = base_offset + (SCREEN_XRESOL * j);
			k = (j * sprite->Xsize) / 8;
			for(i=0;i<(sprite->Xsize);i++)
			{
				if(ptr_src[k + (i>>3)] & (0x80>>(i&7)) )
					ptr_dst[l + i]= 0xFF;
				else
					ptr_dst[l + i]= 0x00;
			}
		}
	}
}

void h_line(int y_pos,unsigned short val)
{
	unsigned char *ptr_dst;
	int i,ptroffset;

	ptr_dst = screen_buffer;
	ptroffset = SCREEN_XRESOL * y_pos;

	for(i=0;i< (SCREEN_XRESOL/2);i++)
	{
		ptr_dst[ptroffset + (i*2)]=(val>>8)&0xFF;
		ptr_dst[ptroffset + (i*2) + 1]=(val)&0xFF;
	}
}

void box(int x_p1,int y_p1,int x_p2,int y_p2,unsigned short fillval,unsigned char fill)
{
	unsigned short *ptr_dst;
	int i,j,ptroffset,x_size;

	ptr_dst=(unsigned short*)screen_buffer;

	x_size=(x_p2-x_p1);

	ptroffset = SCREEN_XRESOL * y_p1;
	for(j=0;j<(y_p2-y_p1);j++)
	{
		for(i=0;i<x_size/2;i++)
		{
			ptr_dst[ptroffset+(i*2)]  =(fillval>>8);
			ptr_dst[ptroffset+(i*2)+1]=(fillval)&0xFF;
		}
		ptroffset = SCREEN_XRESOL * (y_p1+j);
	}

}

void invert_line(int x_pos,int y_pos)
{
	int i,j;
	unsigned char *ptr_dst;
	int ptroffset;

	if( y_pos < SCREEN_YRESOL && x_pos < SCREEN_XRESOL )
	{

		for(j=0;j<8;j++)
		{
			ptr_dst=(unsigned char*)screen_buffer;
			ptroffset=(SCREEN_XRESOL* (y_pos + j))+x_pos;

			for(i=0;i<SCREEN_XRESOL;i++)
			{
				ptr_dst[ptroffset+i]=ptr_dst[ptroffset+i]^0xFF;
			}
		}
	}
}

void save_box()
{
	//memcpy(screen_buffer_backup,&screen_buffer[160*70], 8*1024);
}

void restore_box()
{
//	memcpy(&screen_buffer[160*70],screen_buffer_backup, 8*1024);
}

void reboot()
{
	if(sdl_timer_id)
		SDL_RemoveTimer(sdl_timer_id);
	SDL_Quit();

	close_disk_access();

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

#ifdef DEBUG

void dbg_printf(char * chaine, ...)
{
	va_list marker;
	va_start( marker, chaine );

	vprintf(chaine,marker);

	va_end( marker );
}

#endif

void lockup()
{
	#ifdef DEBUG
	dbg_printf("lockup : Sofware halted...\n");
	#endif

	sleep(2);

	if(sdl_timer_id)
		SDL_RemoveTimer(sdl_timer_id);

	close_disk_access();

	SDL_Quit();

	exit(0);

	for(;;);
}

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,512);
	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,512);

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
							while( argv[param][i] )
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
	printf("  -verbose\t\t\t\t: Verbose mode\n");
	printf("  -disk:[path]\t\t\t\t: Path to the drive to mount \n");
	printf("  -getslots:[filename.txt]\t\t: Get the slot list\n");
	printf("  -setslots:[filename.txt]\t\t: Set the slot list\n");
	printf("  -fixslots\t\t\t\t: Fix the bad slot(s)\n");
	printf("  -populateslots\t\t\t: Scan all supported file and auto add them into the slots\n");
	printf("  -clearslots\t\t\t\t: Clear the slots\n");
	printf("\n");
}

int process_command_line(int argc, char* argv[])
{
	char inoutfile[512];

#ifdef WIN32
	hMassStorage = INVALID_HANDLE_VALUE;
#else
	hMassStorage = 0;
#endif

	printf("HxC Floppy Emulator : HxC Floppy Emulator File selector\n");
	printf("Copyright (C) 2006-2017 Jean-Francois DEL NERO\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions;\n\n");
	printf("%s -help to get the command line options\n\n",argv[0]);

	dev_path[0] = 0;

	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}
	else
	{
		isOption(argc,argv,"disk",(char*)&dev_path);

		inoutfile[0] = 0;
		if(isOption(argc,argv,"getslots",(char*)&inoutfile))
		{
			if(strlen(dev_path))
			{
				generate_slot_list(inoutfile,0);
				return 1;
			}
		}

		if(isOption(argc,argv,"setslots",(char*)&inoutfile))
		{
			if(strlen(dev_path))
			{
				insert_slot_list(inoutfile);
				return 1;
			}
		}

		if(isOption(argc,argv,"fixslots",0))
		{
			if(strlen(dev_path))
			{
				generate_slot_list(0,1);
				return 1;
			}
		}

		if(isOption(argc,argv,"clearslots",0))
		{
			if(strlen(dev_path))
			{
				clear_all_slots();
				return 1;
			}
		}

		if(isOption(argc,argv,"populateslots",0))
		{
			if(strlen(dev_path))
			{
				auto_populate_slots();
				return 1;
			}
		}

		if(!strlen(dev_path))
		{
			printf("\nDisk path not specify !\n");
			printf("Please use the -disk:[path] option to mount your SD/USB\n\n");
			return 1;
		}

		return 0;
	}

	return 1;
}
