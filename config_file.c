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

#ifdef CORTEX_FW_SUPPORT
#include "cortex_cfg_file.h"
#endif

#include "conf.h"
#include "ui_context.h"

#include "gui_utils.h"
#include "hxcfeda.h"

#include "hal.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

#include "fectrl.h"

#include "media_access.h"

#include "errors_def.h"

extern disk_in_drive_v2 disks_slots[MAX_NUMBER_OF_SLOT];
unsigned char cfgfile_header[512];
FL_FILE * cfg_file_handle;

void setcfg_backgroundcolor(int color)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	cfgfile_ptr->background_color = color;
}

int getcfg_backgroundcolor()
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	return cfgfile_ptr->background_color;
}

int read_hxc_cfg_file(ui_context * ctx,unsigned char * cfgfile_header)
{
	int ret;
	unsigned short number_of_slots;
	unsigned short i,d,slot_offset;
	short sector_offset,last_sector_offset;
	int file_cfg_size;
	unsigned char temp_sector[512];
	disk_in_drive * disk;
	cfgfile * cfgfile_ptr;

	#ifdef DEBUG
	dbg_printf("enter read_cfg_file\n");
	#endif

	memset((void*)&disks_slots,0,sizeof(disks_slots));

	ret = ERR_NO_ERROR;

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (cfg_file_handle)
	{
		fl_fseek( cfg_file_handle, 0, SEEK_END );
		file_cfg_size = fl_ftell( cfg_file_handle );
		fl_fseek( cfg_file_handle, 0, SEEK_SET );

		#ifdef DEBUG
		dbg_printf("config size file : %d\n", file_cfg_size);
		#endif

		cfgfile_ptr=(cfgfile * )cfgfile_header;

		fl_fread(cfgfile_header, 1, 512 , cfg_file_handle);

		if( !strncmp(cfgfile_ptr->signature,"HXCFECFGV",9) )
		{
			switch(cfgfile_ptr->signature[9])
			{
				case '1':

					#ifdef DEBUG
					dbg_printf("V1 config file format\n");
					#endif

					ctx->cfg_file_format_version = 1;
					ctx->number_of_drive = 2;
					ctx->config_file_number_max_of_slot = ( ( file_cfg_size - 0x400 ) / (64 * 2) ) - 1;

					if( ctx->config_file_number_max_of_slot > ( MAX_NUMBER_OF_SLOT / 2 ) )
						ctx->config_file_number_max_of_slot = ( MAX_NUMBER_OF_SLOT / 2 );

					number_of_slots = cfgfile_ptr->number_of_slot;
					if( number_of_slots > ( MAX_NUMBER_OF_SLOT / 2 ) )
						number_of_slots = ( MAX_NUMBER_OF_SLOT / 2 ) - 1;

					if( number_of_slots > ctx->config_file_number_max_of_slot )
						number_of_slots = (unsigned short) (ctx->config_file_number_max_of_slot - 1);

					ctx->number_of_slots = number_of_slots;
					fl_fseek(cfg_file_handle , 1024 , SEEK_SET);

					memset(ctx->change_map,0,512);
					memset(ctx->slot_map,0,512);
					fl_fread(temp_sector, 1, 512 , cfg_file_handle);

					i=1;
					while(i<number_of_slots)
					{
						if(!(i&3))
						{
							fl_fread(temp_sector, 1, 512 , cfg_file_handle);
						}

						disk = (disk_in_drive *)&temp_sector[(i&3)*128];
						disks_slots[i*2].attributes       = disk->DirEnt.attributes;
						disks_slots[i*2].firstCluster     = disk->DirEnt.firstCluster;
						disks_slots[i*2].size             = disk->DirEnt.size;
						memcpy(&disks_slots[i*2].name,      disk->DirEnt.longName,17);
						disks_slots[i*2].name[17]         = 0;
						memcpy(&disks_slots[i*2].type,&disk->DirEnt.name[8],3);

						disk = (disk_in_drive *)&temp_sector[((i&3)*128)+64];
						disks_slots[(i*2)+1].attributes   = disk->DirEnt.attributes;
						disks_slots[(i*2)+1].firstCluster = disk->DirEnt.firstCluster;
						disks_slots[(i*2)+1].size         = disk->DirEnt.size;
						memcpy(&disks_slots[(i*2)+1].name,  disk->DirEnt.longName,17);
						disks_slots[(i*2)+1].name[17]     = 0;
						memcpy(&disks_slots[(i*2)+1].type,&disk->DirEnt.name[8],3);

						ctx->slot_map[i>>3] |= (0x80 >> (i&7));
						ctx->change_map[i>>3] |= (0x80 >> (i&7));
						i++;
					};
				break;

				case '2':
					#ifdef DEBUG
					dbg_printf("V2 config file format\n");
					#endif

					ctx->cfg_file_format_version = 2;

					ctx->number_of_drive = ENDIAN_32BIT( cfgfile_ptr->number_of_drive_per_slot );
					ctx->slots_position = ENDIAN_32BIT( cfgfile_ptr->slots_position );
					ctx->config_file_number_max_of_slot = ENDIAN_32BIT( cfgfile_ptr->max_slot_number );

					if( ctx->config_file_number_max_of_slot > (MAX_NUMBER_OF_SLOT / ctx->number_of_drive) )
						ctx->config_file_number_max_of_slot = (MAX_NUMBER_OF_SLOT / ctx->number_of_drive);

					memset(ctx->change_map,0,512);

					number_of_slots = (unsigned short) ENDIAN_32BIT( cfgfile_ptr->max_slot_number );
					fl_fseek(cfg_file_handle , 512 * ENDIAN_32BIT(cfgfile_ptr->slots_map_position) , SEEK_SET);
					fl_fread(ctx->slot_map, 1, 512 , cfg_file_handle);

					last_sector_offset = -1;
					i = 0;
					do
					{
						if( ctx->slot_map[i>>3] & (0x80 >> (i&7)) )
						{
							sector_offset = (short) ( ( i * 64 * ctx->number_of_drive ) / 512 );
							if(last_sector_offset != sector_offset )
							{
								if( sector_offset - last_sector_offset != 1 )
								{
									fl_fseek(cfg_file_handle , 512 * (sector_offset + ENDIAN_32BIT(cfgfile_ptr->slots_position) ), SEEK_SET);
								}
								fl_fread(temp_sector, 1, 512 , cfg_file_handle);
								last_sector_offset = sector_offset;
							}

							d = 0;
							while( d < ctx->number_of_drive )
							{
								slot_offset = (unsigned short) ( ( i * 64 * ctx->number_of_drive ) + ( 64 * d ) ) % 512;
								if( ( (i*ctx->number_of_drive) + d ) < MAX_NUMBER_OF_SLOT )
								{
									memcpy( &disks_slots[ (i*ctx->number_of_drive) + d ],
											&temp_sector[slot_offset],
											sizeof(disk_in_drive_v2));
								}

								d++;
							}
						}
						i++;
					}while(i<number_of_slots);
				break;

				default:
					#ifdef DEBUG
					dbg_printf("Unknown config file version !\n");
					#endif

					ctx->cfg_file_format_version = 0;

					ret = -ERR_CONFIG_FILE_VERSION;
				break;
			}
		}
		else
		{
			ret = -ERR_CONFIG_FILE_SIGN;
		}
	}
	else
	{
		ret = -ERR_CONFIG_FILE_ACCESS;
	}

	#ifdef DEBUG
	dbg_printf("leave read_cfg_file : %d\n",ret);
	#endif

	return ret;
}

#ifdef CORTEX_FW_SUPPORT
int read_cortex_cfg_file(ui_context * ctx,unsigned char * cfgfile_header)
{
	int ret;
	unsigned short number_of_slots;
	unsigned short i;
	unsigned char temp_sector[512];
	Cortex_disk_in_drive * cortex_disk;

	Cortex_cfgfile * cfgfile_ptr;

	memset((void*)&disks_slots,0,sizeof(disks_slots));
	memset(ctx->change_map,0,512);
	memset(ctx->slot_map,0,512);

	ret = ERR_NO_ERROR;

	cfg_file_handle = fl_fopen("/SELECTOR.ADF", "r");
	if (cfg_file_handle)
	{
		cfgfile_ptr = (Cortex_cfgfile * )cfgfile_header;

		fl_fseek(cfg_file_handle , 15*(512*11*2) , SEEK_SET);

		fl_fread(cfgfile_header, 1, 512 , cfg_file_handle);

		ctx->config_file_number_max_of_slot = MAX_NUMBER_OF_SLOT;
		ctx->number_of_drive = 1;

		number_of_slots = ENDIAN_16BIT(cfgfile_ptr->number_of_slot);

		if( number_of_slots > ctx->config_file_number_max_of_slot )
			number_of_slots = (unsigned short) (ctx->config_file_number_max_of_slot - 1);

		ctx->number_of_slots = number_of_slots;

		fl_fseek(cfg_file_handle , (15*(512*11*2))+(2*512) , SEEK_SET);
		i=1;
		fl_fread(temp_sector, 1, 512 , cfg_file_handle);
		do
		{
			if(!(i&3))
			{
				fl_fread(temp_sector, 1, 512 , cfg_file_handle);
			}

			cortex_disk = (Cortex_disk_in_drive *)&temp_sector[(i&3)*128];

			disks_slots[i].attributes       = cortex_disk->DirEnt.attributes;
			disks_slots[i].firstCluster     = cortex_disk->DirEnt.firstCluster;
			disks_slots[i].size             = cortex_disk->DirEnt.size;
			memcpy(&disks_slots[i].name,      cortex_disk->DirEnt.longName,41);
			disks_slots[i].name[41]         = 0;
			memcpy(&disks_slots[i].type,&cortex_disk->DirEnt.name[8],3);

			ctx->slot_map[i>>3] |= (0x80 >> (i&7));

			i++;
		}while(i<number_of_slots);

		fl_fclose(cfg_file_handle);

		return ret;
	}
	else
	{
		return -ERR_CONFIG_FILE_ACCESS;
	}
}
#endif

int read_cfg_file(ui_context * ctx,unsigned char * cfgfile_header)
{
	if( ctx->firmware_type == HXC_LEGACY_FIRMWARE || ctx->firmware_type == HXC_CLONE_FIRMWARE )
	{
		hxc_printf_box(ctx,"Reading HXCSDFE.CFG ...");
		return read_hxc_cfg_file(ctx,cfgfile_header);
	}
	else
	{
#ifdef CORTEX_FW_SUPPORT
		if(ctx->firmware_type == CORTEX_FIRMWARE)
		{
			hxc_printf_box(ctx,"Reading SELECTOR.ADF ...");
			return read_cortex_cfg_file(ctx,cfgfile_header);
		}
#endif
	}

	return -ERR_BAD_DRIVE_ID;
}

int save_hxc_cfg_file(ui_context * ctx,unsigned char * sdfecfg_file, int pre_selected_slot)
{
	int ret;

	unsigned char number_of_slot,slot_index;
	unsigned short i,j,sect_nb;
	cfgfile * cfgfile_ptr;
	uint32_t  floppyselectorindex;
	disk_in_drive * disk;
	unsigned char temp_buf[512];

	#ifdef DEBUG
	dbg_printf("enter save_cfg_file\n");
	#endif

	ret = ERR_NO_ERROR;

	if (cfg_file_handle)
	{
		switch(ctx->cfg_file_format_version)
		{
			case 1:
				#ifdef DEBUG
				dbg_printf("V1 config file format\n");
				#endif

				cfgfile_ptr = (cfgfile * )sdfecfg_file;

				number_of_slot = 1;

				if(pre_selected_slot>=0)
					slot_index = pre_selected_slot;
				else
					slot_index = 1;

				floppyselectorindex=128;                      // Fisrt slot offset
				memset( temp_buf,0,512);                      // Clear the sector
				sect_nb=2;                                    // Slots Sector offset

				i = 1;
				do
				{
					if( ctx->slot_map[i>>3] & (0x80 >> (i&7)) )            // Valid slot found
					{
						// Drive A
						disk = (disk_in_drive *)&temp_buf[floppyselectorindex];
						memset(disk,0,sizeof(disk_in_drive));
						disk->DirEnt.attributes = disks_slots[i*2].attributes;
						disk->DirEnt.firstCluster = disks_slots[i*2].firstCluster;
						disk->DirEnt.size = disks_slots[i*2].size;

						memset(disk->DirEnt.longName,0,17);
						j = 0;
						while( j<16 && disks_slots[i*2].name[j] )
						{
							disk->DirEnt.longName[j] = disks_slots[i*2].name[j];
							j++;
						}

						memset(disk->DirEnt.name,' ',12);
						disk->DirEnt.name[11] = 0;
						j = 0;
						while( j<8 && disks_slots[i*2].name[j] && disks_slots[i*2].name[j]!='.')
						{
							disk->DirEnt.name[j] = disks_slots[i*2].name[j];
							j++;
						}

						memcpy(&disk->DirEnt.name[8],&disks_slots[i*2].type,3);

						// Drive B
						disk = (disk_in_drive *)&temp_buf[floppyselectorindex + 64];
						memset(disk,0,sizeof(disk_in_drive));
						disk->DirEnt.attributes = disks_slots[(i*2)+1].attributes;
						disk->DirEnt.firstCluster = disks_slots[(i*2)+1].firstCluster;
						disk->DirEnt.size = disks_slots[(i*2)+1].size;

						memset(disk->DirEnt.longName,0,17);
						j = 0;
						while( j<16 && disks_slots[(i*2)+1].name[j] )
						{
							disk->DirEnt.longName[j] = disks_slots[(i*2)+1].name[j];
							j++;
						}

						memset(disk->DirEnt.name,' ',12);
						disk->DirEnt.name[11] = 0;
						j = 0;
						while( j<8 && disks_slots[(i*2)+1].name[j] && disks_slots[(i*2)+1].name[j]!='.')
						{
							disk->DirEnt.name[j] = disks_slots[(i*2)+1].name[j];
							j++;
						}

						memcpy(&disk->DirEnt.name[8],&disks_slots[(i*2)+1].type,3);

						//Next slot...
						number_of_slot++;
						floppyselectorindex=(floppyselectorindex+128)&0x1FF;

						if(!(number_of_slot&0x3))                // Need to change to the next sector
						{
							// Save the sector
							if (fl_fswrite((unsigned char*)temp_buf, 1,sect_nb, cfg_file_handle) != 1)
							{
								return -ERR_WRITE_FILE_ACCESS;
							}
							// Next sector
							sect_nb++;
							memset( temp_buf,0,512);                  // Clear the next sector
						}
					}

					i++;
				}while(i<ctx->config_file_number_max_of_slot);

				if(number_of_slot&0x3)
				{
					if (fl_fswrite((unsigned char*)temp_buf, 1,sect_nb, cfg_file_handle) != 1)
					{
						return -ERR_WRITE_FILE_ACCESS;
					}
				}

				if( slot_index >= number_of_slot )
				{
					slot_index = number_of_slot - 1;
				}

				fl_fseek(cfg_file_handle , 0 , SEEK_SET);

				// Update the file header

				cfgfile_ptr->number_of_slot = number_of_slot;
				cfgfile_ptr->slot_index = slot_index;

				if (fl_fswrite((unsigned char*)sdfecfg_file, 1,0, cfg_file_handle) != 1)
				{
					#ifdef DEBUG
					dbg_printf("fl_fswrite error : header %d !\n",0);
					#endif
					return -ERR_WRITE_FILE_ACCESS;
				}


			break;

			case 2:
				#ifdef DEBUG
				dbg_printf("V2 config file format\n");
				#endif

				number_of_slot = 1;

				cfgfile_ptr=(cfgfile * )sdfecfg_file;

				slot_index = 1;

				i = 1;
				do
				{
					if( (ctx->change_map[i>>3] & (0x80 >> (i&7))) && ( (i*ctx->number_of_drive) < MAX_NUMBER_OF_SLOT ) )   // Is the slot modified ?
					{
						// Yes, save the modified sector
						floppyselectorindex=(64*ctx->number_of_drive)*i;

						sect_nb = (unsigned short) ENDIAN_32BIT(cfgfile_ptr->slots_position) + (floppyselectorindex >> 9);
						if (fl_fswrite( ((unsigned char*)&disks_slots) + (floppyselectorindex & ~0x1FF) , 1, sect_nb, cfg_file_handle) != 1)
						{
							#ifdef DEBUG
							dbg_printf("fl_fswrite error : slot sect %d !\n",sect_nb);
							#endif

							return -ERR_WRITE_FILE_ACCESS;

						}

						// And clear the modified flags for all slots into this sector.
						j = 0;
						do
						{
							ctx->change_map[(i+j)>>3] &= ~(0x80 >> ((i+j)&7));
							j++;
						}while( ((64*ctx->number_of_drive)*(i+j)) & 0x1FF );
					}

					i++;
				}while( i < ctx->config_file_number_max_of_slot );

				// Update the map
				if (fl_fswrite((unsigned char*)&ctx->slot_map, 1, ENDIAN_32BIT(cfgfile_ptr->slots_map_position), cfg_file_handle) != 1)
				{
					#ifdef DEBUG
					dbg_printf("fl_fswrite error : map sect %d !\n",ENDIAN_32BIT(cfgfile_ptr->slots_map_position));
					#endif
					return -ERR_WRITE_FILE_ACCESS;
				}

				fl_fseek(cfg_file_handle , 0 , SEEK_SET);

				// Update the file header

				if(pre_selected_slot>=0)
					cfgfile_ptr->cur_slot_number = ENDIAN_32BIT(pre_selected_slot);
				else
					cfgfile_ptr->cur_slot_number = ENDIAN_32BIT(1);

				cfgfile_ptr->slot_index = 0;

				if (fl_fswrite((unsigned char*)sdfecfg_file, 1,0, cfg_file_handle) != 1)
				{
					#ifdef DEBUG
					dbg_printf("fl_fswrite error : header %d !\n",0);
					#endif
					return -ERR_WRITE_FILE_ACCESS;
				}
			break;
		}
	}
	else
	{
		return -ERR_INVALID_HANDLER;
	}

	#ifdef DEBUG
	dbg_printf("leave save_cfg_file\n");
	#endif

	return ret;
}

#ifdef CORTEX_FW_SUPPORT
int save_cortex_cfg_file(ui_context * ctx,unsigned char * sdfecfg_file, int pre_selected_slot)
{
	int ret;
	unsigned int number_of_slot,slot_index;
	unsigned int sect_nb;
	unsigned int i;
	Cortex_cfgfile * cfgfile_ptr;
	Cortex_disk_in_drive * cortex_disk;
	unsigned int floppyselectorindex;
	unsigned char writeneeded;
	FL_FILE *file;
	unsigned char temp_buf[512];

	ret = ERR_NO_ERROR;
	writeneeded = 0;

	file = fl_fopen("/SELECTOR.ADF", "r");
	if (file)
	{
		number_of_slot=1;
		slot_index=1;
		i=1;

		memset( temp_buf,0,512);                           // Clear the sector

		floppyselectorindex = 128;                         // First slot offset
		sect_nb=2;                                         // Slots Sector offset

		do
		{
			if( ctx->slot_map[i>>3] & (0x80 >> (i&7)) )    // Valid slot found
			{
				// Copy it to the actual file sector
				cortex_disk = (Cortex_disk_in_drive *)&temp_buf[floppyselectorindex];

				cortex_disk->DirEnt.attributes = disks_slots[i].attributes;
				cortex_disk->DirEnt.firstCluster = disks_slots[i].firstCluster;
				cortex_disk->DirEnt.size = disks_slots[i].size;
				memcpy(&cortex_disk->DirEnt.longName, &disks_slots[i].name, 40);
				cortex_disk->DirEnt.longName[40] = 0;

				memset(&temp_buf[floppyselectorindex+64],0,sizeof(Cortex_disk_in_drive));

				if( cortex_disk->DirEnt.size )
					slot_index = i;
			}
			else
			{
				memset(&temp_buf[floppyselectorindex],0,sizeof(Cortex_disk_in_drive));
				memset(&temp_buf[floppyselectorindex+64],0,sizeof(Cortex_disk_in_drive));
			}

			if( ctx->change_map[i>>3] & (0x80 >> (i&7)) )
			{
				ctx->change_map[i>>3] ^= (0x80 >> (i&7));
				writeneeded = 0xFF;
			}

			//Next slot...
			number_of_slot++;
			floppyselectorindex = (floppyselectorindex+128) & 0x1FF;

			if((!(number_of_slot&0x3)))                // Need to change to the next sector
			{
				if(writeneeded)
				{
					// Save the sector
					if (fl_fswrite((unsigned char*)temp_buf, 1,330+sect_nb, file) != 1)
					{
						return -ERR_WRITE_FILE_ACCESS;
					}
					// Next sector
					writeneeded = 0x00;
				}

				sect_nb++;
				memset( temp_buf,0,512);                  // Clear the next sector
			}

			i++;
		}while( i < MAX_NUMBER_OF_SLOT );

		if((number_of_slot&0x3) && writeneeded )
		{
			writeneeded = 0x00;
			if (fl_fswrite((unsigned char*)temp_buf, 1,330+sect_nb, file) != 1)
			{
				return -ERR_WRITE_FILE_ACCESS;
			}
		}

		number_of_slot = slot_index + 1;

		fl_fseek(file , 0 , SEEK_SET);

		// Update the file header

		if(pre_selected_slot>=0)
			slot_index = pre_selected_slot;
		else
			slot_index = 1;

		cfgfile_ptr = (Cortex_cfgfile * )cfgfile_header;
		cfgfile_ptr->number_of_slot = ENDIAN_16BIT(number_of_slot);
		cfgfile_ptr->slot_index = ENDIAN_16BIT(slot_index);
		cfgfile_ptr->update_cnt = ENDIAN_16BIT( (ENDIAN_16BIT(cfgfile_ptr->update_cnt) + 1 ) );

		if (fl_fswrite((unsigned char*)cfgfile_header, 1,330, file) != 1)
		{
			return -ERR_WRITE_FILE_ACCESS;
		}
	}
	else
	{
		return -ERR_CONFIG_FILE_ACCESS;
	}

	// Close file
	fl_fclose(file);

	return ret;
}
#endif

int save_cfg_file(ui_context * ctx,unsigned char * sdfecfg_file, int pre_selected_slot)
{
	if( ctx->firmware_type == HXC_LEGACY_FIRMWARE || ctx->firmware_type == HXC_CLONE_FIRMWARE )
	{
		return save_hxc_cfg_file(ctx,sdfecfg_file, pre_selected_slot);
	}
	else
	{
#ifdef CORTEX_FW_SUPPORT
		return save_cortex_cfg_file(ctx,sdfecfg_file, pre_selected_slot);
#endif
	}

	return -ERR_BAD_DRIVE_ID;
}
