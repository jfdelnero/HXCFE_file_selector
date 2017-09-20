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

#include "gui_utils.h"
#include "cfg_file.h"
#include "hxcfeda.h"

#include "hardware.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

#include "msg_txt.h"

#include "version.h"

#include "conf.h"
#include "ui_context.h"

#include "fectrl.h"
#include "config_file.h"
#include "media_access.h"

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

char read_cfg_file(ui_context * uicontext,unsigned char * cfgfile_header)
{
	char ret;
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

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (!cfg_file_handle)
	{
		hxc_printf_box("ERROR: Can't open HXCSDFE.CFG !");
		lockup();
	}

	ret=0;
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

					uicontext->cfg_file_format_version = 1;
					uicontext->number_of_drive = 2;
					uicontext->config_file_number_max_of_slot = ( ( file_cfg_size - 0x400 ) / (64 * 2) ) - 1;

					if( uicontext->config_file_number_max_of_slot > ( MAX_NUMBER_OF_SLOT / 2 ) )
						uicontext->config_file_number_max_of_slot = ( MAX_NUMBER_OF_SLOT / 2 );

					number_of_slots = cfgfile_ptr->number_of_slot;
					if( number_of_slots > ( MAX_NUMBER_OF_SLOT / 2 ) )
						number_of_slots = ( MAX_NUMBER_OF_SLOT / 2 ) - 1;

					if( number_of_slots > uicontext->config_file_number_max_of_slot )
						number_of_slots = (unsigned short) (uicontext->config_file_number_max_of_slot - 1);

					uicontext->number_of_slots = number_of_slots;
					fl_fseek(cfg_file_handle , 1024 , SEEK_SET);

					memset(uicontext->change_map,0,512);
					memset(uicontext->slot_map,0,512);
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

						uicontext->slot_map[i>>3] |= (0x80 >> (i&7));
						uicontext->change_map[i>>3] |= (0x80 >> (i&7));
						i++;
					};
				break;

				case '2':
					#ifdef DEBUG
					dbg_printf("V2 config file format\n");
					#endif

					uicontext->cfg_file_format_version = 2;

					uicontext->number_of_drive = ENDIAN_32BIT( cfgfile_ptr->number_of_drive_per_slot );
					uicontext->slots_position = ENDIAN_32BIT( cfgfile_ptr->slots_position );
					uicontext->config_file_number_max_of_slot = ENDIAN_32BIT( cfgfile_ptr->max_slot_number );

					if( uicontext->config_file_number_max_of_slot > (MAX_NUMBER_OF_SLOT / uicontext->number_of_drive) )
						uicontext->config_file_number_max_of_slot = (MAX_NUMBER_OF_SLOT / uicontext->number_of_drive);

					memset(uicontext->change_map,0,512);

					number_of_slots = (unsigned short) ENDIAN_32BIT( cfgfile_ptr->max_slot_number );
					fl_fseek(cfg_file_handle , 512 * ENDIAN_32BIT(cfgfile_ptr->slots_map_position) , SEEK_SET);
					fl_fread(uicontext->slot_map, 1, 512 , cfg_file_handle);

					last_sector_offset = -1;
					i = 0;
					do
					{
						if( uicontext->slot_map[i>>3] & (0x80 >> (i&7)) )
						{
							sector_offset = (short) ( ( i * 64 * uicontext->number_of_drive ) / 512 );
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
							while( d < uicontext->number_of_drive )
							{
								slot_offset = (unsigned short) ( ( i * 64 * uicontext->number_of_drive ) + ( 64 * d ) ) % 512;
								if( ( (i*uicontext->number_of_drive) + d ) < MAX_NUMBER_OF_SLOT )
								{
									memcpy( &disks_slots[ (i*uicontext->number_of_drive) + d ],
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

					uicontext->cfg_file_format_version = 0;

					ret=3;
				break;
			}
		}
		else
		{
			ret=2;
		}

	}
	else
	{
		ret=1;
	}

	if(ret)
	{
		hxc_printf_box("ERROR: Access HXCSDFE.CFG file failed! [%d]",ret);
		lockup();
	}

	#ifdef DEBUG
	dbg_printf("leave read_cfg_file : %d\n",ret);
	#endif

	return ret;
}

char save_cfg_file(ui_context * uicontext,unsigned char * sdfecfg_file, int pre_selected_slot)
{
	unsigned char number_of_slot,slot_index;
	unsigned char ret;
	unsigned short i,j,sect_nb;
	cfgfile * cfgfile_ptr;
	uint32_t  floppyselectorindex;
	disk_in_drive * disk;
	unsigned char temp_buf[512];

	#ifdef DEBUG
	dbg_printf("enter save_cfg_file\n");
	#endif

	ret=0;
	if (cfg_file_handle)
	{
		switch(uicontext->cfg_file_format_version)
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
					if( uicontext->slot_map[i>>3] & (0x80 >> (i&7)) )            // Valid slot found
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
								hxc_printf_box("ERROR: Write file failed!");
								ret=1;
							}
							// Next sector
							sect_nb++;
							memset( temp_buf,0,512);                  // Clear the next sector
						}
					}

					i++;
				}while(i<uicontext->config_file_number_max_of_slot);

				if(number_of_slot&0x3)
				{
					if (fl_fswrite((unsigned char*)temp_buf, 1,sect_nb, cfg_file_handle) != 1)
					{
						hxc_printf_box("ERROR: Write file failed!");
						ret=1;
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

					hxc_printf_box("ERROR: Write file failed!");
					ret=1;
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
					if( (uicontext->change_map[i>>3] & (0x80 >> (i&7))) && ( (i*uicontext->number_of_drive) < MAX_NUMBER_OF_SLOT ) )   // Is the slot modified ?
					{
						// Yes, save the modified sector
						floppyselectorindex=(64*uicontext->number_of_drive)*i;

						sect_nb = (unsigned short) ENDIAN_32BIT(cfgfile_ptr->slots_position) + (floppyselectorindex >> 9);
						if (fl_fswrite( ((unsigned char*)&disks_slots) + (floppyselectorindex & ~0x1FF) , 1, sect_nb, cfg_file_handle) != 1)
						{
							#ifdef DEBUG
							dbg_printf("fl_fswrite error : slot sect %d !\n",sect_nb);
							#endif

							hxc_printf_box("ERROR: Write file failed!");
							ret=1;
						}

						// And clear the modified flags for all slots into this sector.
						j = 0;
						do
						{
							uicontext->change_map[(i+j)>>3] &= ~(0x80 >> ((i+j)&7));
							j++;
						}while( ((64*uicontext->number_of_drive)*(i+j)) & 0x1FF );
					}

					i++;
				}while( i < uicontext->config_file_number_max_of_slot );

				// Update the map
				if (fl_fswrite((unsigned char*)&uicontext->slot_map, 1, ENDIAN_32BIT(cfgfile_ptr->slots_map_position), cfg_file_handle) != 1)
				{
					#ifdef DEBUG
					dbg_printf("fl_fswrite error : map sect %d !\n",ENDIAN_32BIT(cfgfile_ptr->slots_map_position));
					#endif

					hxc_printf_box("ERROR: Write file failed!");
					ret=1;
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

					hxc_printf_box("ERROR: Write file failed!");
					ret=1;
				}
			break;
		}
	}
	else
	{
		hxc_printf_box("ERROR: Create file failed!");
		ret=1;
	}

	#ifdef DEBUG
	dbg_printf("leave save_cfg_file\n");
	#endif

	return ret;
}
