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

#include "color_table.h"

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"

#include "../graphx/font.h"

#include "hal.h"

#include "slot_list_gen.h"

#include "hxcfeda.h"

#include "errors_def.h"

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

uint16_t sector_pos[16];

unsigned char keyup;

uint32_t timercnt;

int track_number,number_of_sector;
uint32_t virt_lba;

unsigned char * keys_stat;
unsigned char last_key;
unsigned char cortexfw;

direct_access_status_sector virtual_hxcfe_status;

char dev_path[512];

static unsigned char datacache[512*16];
static unsigned char valid_cache;

extern ui_context g_ui_ctx;

static const char HXC_FW_ID[]="HxCFEDA";
static const char CORTEX_FW_ID[]="CORTEXAD";

static const char VIRT_VERSIONCODE[]="v3.X.X.Xa";
static const char CORTEX_VIRT_VERSIONCODE[]="v1.05a";

void waitus(int centus)
{

}

void waitms(int ms)
{
	SDL_Delay(ms);
}

void waitsec(int secs)
{
	int i;

	for(i=0;i<secs;i++)
	{
		waitms(1000);
	}
}

int jumptotrack(unsigned char t)
{
	track_number = t;
	return ERR_NO_ERROR;
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

			return ERR_NO_ERROR;
		}

		if(locked)
			DeviceIoControl(hMassStorage,(DWORD) FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &dwNotUsed, NULL);
	}

	#else
	if(hMassStorage)
	{
		if( fseeko(hMassStorage,(off_t)lba*(off_t)512,SEEK_SET) )
			return -ERR_MEDIA_WRITE;

		if( fwrite(data,512,1,hMassStorage) != 1 )
			return -ERR_MEDIA_WRITE;

		return ERR_NO_ERROR;
	}
	#endif

	return -ERR_MEDIA_WRITE;
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
			return ERR_NO_ERROR;
		}
	}

	#else
	int ret;

	if(hMassStorage)
	{
		if( fseeko(hMassStorage,(off_t)lba*(off_t)512,SEEK_SET) )
			return -ERR_MEDIA_READ;

		if( fread(data,nbsector*512,1,hMassStorage) != 1 )
			return -ERR_MEDIA_READ;
		else
			return ERR_NO_ERROR;
	}
	#endif

	return -ERR_MEDIA_READ;
}

int writesector(unsigned char sectornum,unsigned char * data)
{
	direct_access_cmd_sector  * da_cmd;

	valid_cache=0;

	if(track_number!=255)
		return -ERR_INVALID_PARAMETER;

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

		return ERR_NO_ERROR;
	}
	else
	{
		if(sectornum > number_of_sector)
		{
			return -ERR_INVALID_PARAMETER;
		}

		return write_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), data);
	}
}

int readsector(unsigned char sectornum,unsigned char * data,unsigned char invalidate_cache)
{
	int ret;

	if(track_number!=255)
		return -ERR_INVALID_PARAMETER;

	if(!sectornum)
	{
		memset(data,0,512);
		memcpy(data,&virtual_hxcfe_status,sizeof(virtual_hxcfe_status));

		return ERR_NO_ERROR;
	}
	else
	{
		if(sectornum > number_of_sector)
			return -ERR_INVALID_PARAMETER;

		ret = ERR_NO_ERROR;

		if(!valid_cache || invalidate_cache)
		{
			ret = read_mass_storage(virtual_hxcfe_status.lba_base + (sectornum - 1), datacache, number_of_sector);
			if( ret == ERR_NO_ERROR )
				valid_cache=0xFF;
		}

		if( ret == ERR_NO_ERROR )
			memcpy((void*)data,&datacache[(sectornum-1)*512],512);

		return ret;
	}
}

int init_fdc(int drive)
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

	if(cortexfw)
	{
		memcpy((char*)&virtual_hxcfe_status.DAHEADERSIGNATURE, CORTEX_FW_ID, strlen(CORTEX_FW_ID));
		memcpy((char*)&virtual_hxcfe_status.FIRMWAREVERSION,   CORTEX_VIRT_VERSIONCODE, strlen(CORTEX_VIRT_VERSIONCODE));
	}
	else
	{
		memcpy((char*)&virtual_hxcfe_status.DAHEADERSIGNATURE, HXC_FW_ID, strlen(HXC_FW_ID));
		memcpy((char*)&virtual_hxcfe_status.FIRMWAREVERSION,   VIRT_VERSIONCODE, strlen(VIRT_VERSIONCODE));
	}

	virt_lba = 0;
	valid_cache = 0;
	number_of_sector = 9;
	jumptotrack(255);

	return ERR_NO_ERROR;
}

void deinit_fdc()
{
	close_disk_access();
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

unsigned char get_char()
{
	int i;

	do
	{
		SDL_PumpEvents();

		i = 0;
		while( char_keysmap[i].function_code != 0xFF )
		{
			if(keys_stat[char_keysmap[i].keyboard_code])
			{
				waitms(100);
				return char_keysmap[i].function_code;
			}

			i++;
		}
	}while(1);
}

unsigned char wait_function_key()
{
	unsigned char key,joy,i,c;
	unsigned char function_code,key_code;

	function_code = FCT_NO_FUNCTION;

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

int update_screen(ui_context * ctx)
{
	unsigned char *buffer_dat;

	buffer_dat = (unsigned char *)bBuffer->pixels;

	SDL_LockSurface(bBuffer);
	memcpy(buffer_dat,screen_buffer,(ctx->SCREEN_XRESOL*ctx->SCREEN_YRESOL));
	SDL_UnlockSurface(bBuffer);

	SDL_BlitSurface( bBuffer, NULL, screen, &rBuffer );
	SDL_UpdateRect( screen, 0, 0, 0, 0 );

	return ERR_NO_ERROR;
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

	update_screen(&g_ui_ctx);

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

int init_display(ui_context * ctx)
{
	int i,buffer_size;

	track_number = 0;

	ctx->SCREEN_XRESOL = 1024;
	ctx->SCREEN_YRESOL = 480;

	ctx->screen_txt_xsize = ctx->SCREEN_XRESOL / FONT_SIZE_X;
	ctx->screen_txt_ysize = ctx->SCREEN_YRESOL / FONT_SIZE_Y;

	buffer_size = ctx->SCREEN_XRESOL * ctx->SCREEN_YRESOL;

	screen_buffer = malloc(buffer_size);
	if(!screen_buffer)
		return -ERR_MEM_ALLOC;

	memset(screen_buffer,0,buffer_size);

	sdl_timer_id = 0;

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER );

	screen = SDL_SetVideoMode( ctx->SCREEN_XRESOL, ctx->SCREEN_YRESOL, 8, SDL_SWSURFACE);

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

	update_screen(ctx);

	init_timer();

	return ERR_NO_ERROR;
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

void print_char8x8(ui_context * ctx, int col, int line, unsigned char c, int mode)
{
	int i,j;
	unsigned char *ptr_dst;
	unsigned char *font;
	unsigned char set_byte;

	ptr_dst = (unsigned char*)screen_buffer;

	if(mode & INVERTED)
		set_byte = 0x00;
	else
		set_byte = 0xFF;

	if(col < ctx->screen_txt_xsize && line < ctx->screen_txt_ysize)
	{
		ptr_dst += (((line<<3)*ctx->SCREEN_XRESOL)+ (col<<3));
		font     = font_data + (c * ((FONT_SIZE_X*FONT_SIZE_Y)/8));

		for(j=0;j<8;j++)
		{
			for(i=0;i<8;i++)
			{
				if( *font & (0x80>>i) )
					ptr_dst[i]= set_byte;
				else
					ptr_dst[i]= set_byte ^ 0xFF;
			}
			font++;
			ptr_dst=ptr_dst + ctx->SCREEN_XRESOL;
		}
	}

}

void clear_line(ui_context * ctx,int line,int mode)
{
	int i;

	for(i=0;i<ctx->screen_txt_xsize;i++)
	{
		print_char8x8(ctx,i,line,' ',mode);
	}
}

void invert_line(ui_context * ctx, int line)
{
	int i,j;
	unsigned char *ptr_dst;
	int ptroffset;

	if( line < ctx->screen_txt_xsize )
	{
		ptr_dst=(unsigned char*)screen_buffer;

		for(j=0;j<8;j++)
		{
			ptroffset=(ctx->SCREEN_XRESOL* ((line<<3) + j));

			for(i=0;i<ctx->SCREEN_XRESOL;i++)
			{
				ptr_dst[ptroffset+i] ^= 0xFF;
			}
		}
	}
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

	waitsec(2);

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
	printf("  -cortex \t\t\t\t: Cortex Firmware mode\n");

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

	cortexfw = 0;
	dev_path[0] = 0;

	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}
	else
	{
		isOption(argc,argv,"disk",(char*)&dev_path);

		if(isOption(argc,argv,"cortex",0))
		{
			cortexfw = 1;
		}

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
			printf("\nMissing SD/USB Disk path !\n");
			printf("Please use the -disk:[path] option to mount your SD/USB\n\n");
			return 1;
		}

		return 0;
	}

	return 1;
}
