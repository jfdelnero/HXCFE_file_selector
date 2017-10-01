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
#include <stdint.h>
#include "keysfunc_defs.h"

#include "cfg_file.h"

#include "conf.h"
#include "ui_context.h"

#include "gui_utils.h"
#include "hxcfeda.h"

#include "hardware.h"
#include "hal.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

#include "fectrl.h"

#include "media_access.h"

#include "errors_def.h"

static uint32_t last_setlbabase;

extern ui_context g_ui_ctx;

volatile unsigned short io_floppy_timeout;

static const char HXC_FW_ID[]="HxCFEDA";

#ifdef CORTEX_FW_SUPPORT
static const char CORTEX_FW_ID[]="CORTEXAD";
#endif

int setlbabase(unsigned long lba)
{
	int ret;
	unsigned char sector[512];
	direct_access_cmd_sector * dacs;

	#ifdef DEBUG
	dbg_printf("setlbabase : 0x%.8X\n",lba);
	#endif

	dacs=(direct_access_cmd_sector  *)sector;

	memset(&sector,0,512);

#ifdef CORTEX_FW_SUPPORT
	if(g_ui_ctx.firmware_type == CORTEX_FIRMWARE)
	{
		strcpy(dacs->DAHEADERSIGNATURE,CORTEX_FW_ID);
	}
	else
	{
		strcpy(dacs->DAHEADERSIGNATURE,HXC_FW_ID);
	}
#else
	strcpy(dacs->DAHEADERSIGNATURE,HXC_FW_ID);
#endif

	dacs->cmd_code=1;
	dacs->parameter_0 = (unsigned char)((lba>>0)&0xFF);
	dacs->parameter_1 = (unsigned char)((lba>>8)&0xFF);
	dacs->parameter_2 = (unsigned char)((lba>>16)&0xFF);
	dacs->parameter_3 = (unsigned char)((lba>>24)&0xFF);
	dacs->parameter_4 = 0xA5;

	ret = writesector( 0,(unsigned char *)&sector);

	return ret;
}

int test_floppy_if()
{
	int ret;
	unsigned char sector[512];
	direct_access_status_sector * dass;

	dass=(direct_access_status_sector *)sector;

	last_setlbabase = 2;
	do
	{
		ret = setlbabase( last_setlbabase );
		if( ret != ERR_NO_ERROR )
			return ret;

		ret = readsector(0,sector,1);
		if( ret != ERR_NO_ERROR )
			return ret;

		#ifdef DEBUG
		dbg_printf("test_floppy_if : %.8X = %.8X ?\n",last_setlbabase,L_INDIAN(dass->lba_base));
		#endif

		if(last_setlbabase!=L_INDIAN(dass->lba_base))
		{
			return -ERR_LBA_CHANGE_FAILURE;
		}

		last_setlbabase--;
	}while(last_setlbabase);

	return ERR_NO_ERROR;
}

int media_init()
{
	int ret;
	unsigned char sector[512];
	direct_access_status_sector * dass;
	int i,count;

	#ifdef DEBUG
	dbg_printf("media_init\n");
	#endif

	last_setlbabase=0xFFFFF000;
	ret = readsector(0,(unsigned char*)&sector,1);

	g_ui_ctx.firmware_type = INVALID_FIRMWARE;

	if(ret == ERR_NO_ERROR)
	{
		dass = (direct_access_status_sector *)sector;

		if(!strcmp(dass->DAHEADERSIGNATURE,HXC_FW_ID))
		{
			g_ui_ctx.firmware_type = HXC_LEGACY_FIRMWARE;
			i = 0;
			count = 0;

			if(dass->FIRMWAREVERSION[0] != 'v' && dass->FIRMWAREVERSION[0] != 'V')
			{
				g_ui_ctx.firmware_type = HXC_CLONE_FIRMWARE;
			}

			while(dass->FIRMWAREVERSION[i] && i < sizeof(dass->FIRMWAREVERSION))
			{
				if(dass->FIRMWAREVERSION[i] == '.')
					count++;

				i++;
			}

			if(count != 3)
				g_ui_ctx.firmware_type = HXC_CLONE_FIRMWARE;
		}

#ifdef CORTEX_FW_SUPPORT
		if(!strncmp(dass->DAHEADERSIGNATURE,CORTEX_FW_ID,strlen(CORTEX_FW_ID)))
		{
			g_ui_ctx.firmware_type = CORTEX_FIRMWARE;
		}
#endif

		if( g_ui_ctx.firmware_type != INVALID_FIRMWARE )
		{
			strncpy(g_ui_ctx.FIRMWAREVERSION,dass->FIRMWAREVERSION,sizeof(g_ui_ctx.FIRMWAREVERSION));
			hxc_printf(&g_ui_ctx,LEFT_ALIGNED|INVERTED,0, g_ui_ctx.screen_txt_ysize - 1,"FW %s",g_ui_ctx.FIRMWAREVERSION);

			ret = test_floppy_if();
			if( ret != ERR_NO_ERROR)
				return ret;

			dass = (direct_access_status_sector *)sector;
			last_setlbabase=0;

			ret = setlbabase(last_setlbabase);
			if( ret != ERR_NO_ERROR )
				return ret;

			#ifdef DEBUG
			dbg_printf("media_init : HxC FE Found\n");
			#endif

			return ERR_NO_ERROR;
		}

		#ifdef DEBUG
		dbg_printf("media_init : HxC FE not detected\n");
		#endif

		return -ERR_BAD_DRIVE_ID;
	}

	#ifdef DEBUG
	dbg_printf("media_init : Media access error\n");
	#endif

	return ret;
}

int media_access_init(int drive)
{
	int ret;
	io_floppy_timeout = 0;

	ret = init_fdc(drive);
	if( ret != ERR_NO_ERROR )
		return ret;

	#ifdef DEBUG
	dbg_printf("init_fdc Done\n");
	#endif

	ret = media_init();
	if( ret == ERR_NO_ERROR )
	{
		#ifdef DEBUG
		dbg_printf("media_init done\n");
		#endif

		// Initialise File IO Library
		fl_init();

		#ifdef DEBUG
		dbg_printf("fl_init done\n");
		#endif

		/* Attach media access functions to library*/
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
		{
			return -ERR_MEDIA_ATTACH;
		}

		#ifdef DEBUG
		dbg_printf("fl_attach_media done\n");
		#endif

		return ERR_NO_ERROR;
	}

	return ret;
}

int media_read(uint32 sector, uint8 *buffer, uint32 sector_count)
{
	int ret;
	uint32 i;
	direct_access_status_sector * dass;

	dass = (direct_access_status_sector *)buffer;

	#ifdef DEBUG
	dbg_printf("media_read sector : 0x%.8X, cnt : %d \n",sector,sector_count);
	#endif

	hxc_printf(&g_ui_ctx,LEFT_ALIGNED,8*79,1,"%c",23);

	for(i=0;i<sector_count;i++)
	{
		do
		{
			if((sector-last_setlbabase)>=8)
			{
				ret = setlbabase(sector);
				if( ret != ERR_NO_ERROR )
					return 0;
			}

			ret = readsector(0,buffer,0); 
			if( ret != ERR_NO_ERROR )
			{
				hxc_printf_box(&g_ui_ctx,"ERROR: Read ERROR ! fsector %d [Err %d]",(sector-last_setlbabase)+1,ret);

				return 0;
			}
			last_setlbabase = L_INDIAN(dass->lba_base);

		}while((sector-L_INDIAN(dass->lba_base))>=8);

		ret = readsector((unsigned char)((sector-last_setlbabase)+1),&buffer[i*512],0);
		if( ret != ERR_NO_ERROR )
		{
			hxc_printf_box(&g_ui_ctx,"ERROR: Read ERROR ! fsector %d [Err %d]",(sector-last_setlbabase)+1,ret);
			return 0;
		}

		sector++;

		#ifdef DEBUG
		print_hex_array(buffer,512);
		#endif
	}

	hxc_print(&g_ui_ctx,LEFT_ALIGNED,8*79,1," ");

	return 1;
}

int media_write(uint32 sector, uint8 *buffer, uint32 sector_count)
{
	int ret;
	uint32 i;

	#ifdef DEBUG
	dbg_printf("media_write : 0x%.8X\n",sector);
	#endif

	hxc_printf(&g_ui_ctx,LEFT_ALIGNED,8*79,1,"%c",23);

	for(i=0;i<sector_count;i++)
	{
		if( sector - last_setlbabase >=8)
		{
			last_setlbabase=sector;
			ret = setlbabase(sector);
			if( ret != ERR_NO_ERROR )
				return 0;
		}

		ret = writesector((unsigned char)((sector-last_setlbabase)+1),buffer);
		if( ret != ERR_NO_ERROR )
		{
			hxc_printf_box(&g_ui_ctx,"ERROR: Write sector ERROR !");
			return 0;
		}

		sector++;
	}
	hxc_print(&g_ui_ctx,LEFT_ALIGNED,8*79,1," ");

	#ifdef DEBUG
	print_hex_array(buffer,512);
	#endif

	return 1;
}
