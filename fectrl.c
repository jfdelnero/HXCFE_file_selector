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

static uint32_t last_setlbabase;

// Config file header sector
static unsigned char cfgfile_header[512];

// Slots buffer
disk_in_drive_v2 disks_slots[MAX_NUMBER_OF_SLOT];

static disk_in_drive_v2_long DirectoryEntry_tab[40];

// File listing pages directory offset buffer
FL_DIR file_list_status;
FL_DIR file_list_status_tab[MAX_PAGES_PER_DIRECTORY];

static struct fs_dir_ent dir_entry;

extern uint16_t SCREEN_XRESOL;
extern uint16_t SCREEN_YRESOL;
extern unsigned char  NUMBER_OF_FILE_ON_DISPLAY;

extern uint32_t timercnt;
extern unsigned char keyup;

volatile unsigned short io_floppy_timeout;
char FIRMWAREVERSION[16];

ui_context g_ui_ctx;

FL_FILE * cfg_file_handle;

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

	strcpy(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;
	dacs->parameter_0 = (unsigned char)((lba>>0)&0xFF);
	dacs->parameter_1 = (unsigned char)((lba>>8)&0xFF);
	dacs->parameter_2 = (unsigned char)((lba>>16)&0xFF);
	dacs->parameter_3 = (unsigned char)((lba>>24)&0xFF);
	dacs->parameter_4 = 0xA5;

	ret = writesector( 0,(unsigned char *)&sector);
	if(!ret)
	{
		hxc_printf_box("ERROR: Write CTRL ERROR !");
		lockup();
	}

	return 0;
}

int test_floppy_if()
{
	unsigned char sector[512];
	direct_access_status_sector * dass;

	dass=(direct_access_status_sector *)sector;

	last_setlbabase = 2;
	do
	{
		setlbabase(last_setlbabase);
		if(!readsector(0,sector,1))
		{
			hxc_printf_box("read sector %d error !",last_setlbabase);
			lockup();
		}

		#ifdef DEBUG
		dbg_printf("test_floppy_if : %.8X = %.8X ?\n",last_setlbabase,L_INDIAN(dass->lba_base));
		#endif

		if(last_setlbabase!=L_INDIAN(dass->lba_base))
		{
			hxc_printf_box("LBA Change Test Failed ! Write Issue ?");
			lockup();
		}

		last_setlbabase--;
	}while(last_setlbabase);

	return 0;
}

int media_init()
{
	unsigned char ret;
	unsigned char sector[512];
	direct_access_status_sector * dass;

	#ifdef DEBUG
	dbg_printf("media_init\n");
	#endif

	last_setlbabase=0xFFFFF000;
	ret=readsector(0,(unsigned char*)&sector,1);

	if(ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
			strncpy(FIRMWAREVERSION,dass->FIRMWAREVERSION,sizeof(FIRMWAREVERSION));
			hxc_printf(LEFT_ALIGNED,0, (uint16_t)(SCREEN_YRESOL - ( 8 + 2 )) ,"FW Ver %s",FIRMWAREVERSION);

			test_floppy_if();

			dass= (direct_access_status_sector *)sector;
			last_setlbabase=0;
			setlbabase(last_setlbabase);

			#ifdef DEBUG
			dbg_printf("media_init : HxC FE Found\n");
			#endif

			return 1;
		}

		hxc_printf_box("Bad signature - HxC Floppy Emulator not found!");

		#ifdef DEBUG
		dbg_printf("media_init : HxC FE not detected\n");
		#endif

		return 0;
	}

	hxc_printf_box("ERROR: Floppy Access error!  [%d]",ret);

	#ifdef DEBUG
	dbg_printf("media_init : Media access error\n");
	#endif

	return 0;
}

int media_read(uint32 sector, uint8 *buffer, uint32 sector_count)
{
	uint32 i;
	direct_access_status_sector * dass;

	dass= (direct_access_status_sector *)buffer;

	#ifdef DEBUG
	dbg_printf("media_read sector : 0x%.8X, cnt : %d \n",sector,sector_count);
	#endif

	hxc_printf(LEFT_ALIGNED,8*79,1,"%c",23);

	for(i=0;i<sector_count;i++)
	{
		do
		{
			if((sector-last_setlbabase)>=8)
			{
				setlbabase(sector);
			}

			if(!readsector(0,buffer,0))
			{
				hxc_printf_box("ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
			}
			last_setlbabase = L_INDIAN(dass->lba_base);

		}while((sector-L_INDIAN(dass->lba_base))>=8);

		if(!readsector((unsigned char)((sector-last_setlbabase)+1),&buffer[i*512],0))
		{
			hxc_printf_box("ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
			lockup();
		}

		sector++;

		#ifdef DEBUG
		print_hex_array(buffer,512);
		#endif
	}

	hxc_print(LEFT_ALIGNED,8*79,1," ");

	return 1;
}

int media_write(uint32 sector, uint8 *buffer, uint32 sector_count)
{
	uint32 i;

	#ifdef DEBUG
	dbg_printf("media_write : 0x%.8X\n",sector);
	#endif

	hxc_printf(LEFT_ALIGNED,8*79,1,"%c",23);

	for(i=0;i<sector_count;i++)
	{
		if( sector - last_setlbabase >=8)
		{
			last_setlbabase=sector;
			setlbabase(sector);
		}

		if(!writesector((unsigned char)((sector-last_setlbabase)+1),buffer))
		{
			hxc_printf_box("ERROR: Write sector ERROR !");
			lockup();
		}

		sector++;
	}
	hxc_print(LEFT_ALIGNED,8*79,1," ");

	#ifdef DEBUG
	print_hex_array(buffer,512);
	#endif

	return 1;
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

void clear_list(unsigned char add)
{
	unsigned char y_pos,i;

	y_pos = FILELIST_Y_POS;
	for(i=0;i<NUMBER_OF_FILE_ON_DISPLAY+add;i++)
	{
		clear_line(y_pos,0);
		y_pos=y_pos+8;
	}
}

void displayFolder(ui_context * uicontext)
{
	int i;
	hxc_print(LEFT_ALIGNED,0,CURDIR_Y_POS,(char*)cur_folder_msg);

	for(i=15*8;i<SCREEN_XRESOL;i=i+8)
		hxc_print(LEFT_ALIGNED,(uint16_t)i,(uint16_t)CURDIR_Y_POS," ");

	if(strlen(uicontext->currentPath)<32)
		hxc_printf(LEFT_ALIGNED,15*8,CURDIR_Y_POS,"%s",uicontext->currentPath);
	else
		hxc_printf(LEFT_ALIGNED,15*8,CURDIR_Y_POS,"...%s    ",&uicontext->currentPath[strlen(uicontext->currentPath)-32]);
}

void enter_sub_dir(ui_context * uicontext,disk_in_drive_v2_long *disk_ptr)
{
	int currentPathLength;
	char folder[128+1];
	unsigned char c;
	int i;
	int old_index;

	old_index = strlen( uicontext->currentPath );

	if ( !strncmp(disk_ptr->name,"..", 2) )
	{
		currentPathLength = strlen( uicontext->currentPath ) - 1;
		do
		{
			uicontext->currentPath[ currentPathLength ] = 0;
			currentPathLength--;
		}while ( uicontext->currentPath[ currentPathLength ] != (unsigned char)'/' );
	}
	else
	{
		if((disk_ptr->name[0] != (unsigned char)'.'))
		{
			for (i=0; i < 128; i++ )
			{
				c = disk_ptr->name[i];
				if ( ( c >= (32+0) ) && (c <= 127) )
				{
					folder[i] = c;
				}
				else
				{
					folder[i] = 0;
					i = 128;
				}
			}

			currentPathLength = strlen( uicontext->currentPath );

			if( uicontext->currentPath[ currentPathLength-1] != '/')
			strcat( uicontext->currentPath, "/" );

			strcat( uicontext->currentPath, folder );
		}
	}

	displayFolder(uicontext);

	uicontext->selectorpos=0;

	if(!fl_opendir(uicontext->currentPath, &file_list_status))
	{
		uicontext->currentPath[old_index]=0;
		fl_opendir(uicontext->currentPath, &file_list_status);
		displayFolder(uicontext);
	}

	for(i=0;i<MAX_PAGES_PER_DIRECTORY;i++)
	{
		memcpy(&file_list_status_tab[i],&file_list_status ,sizeof(FL_DIR));
	}

 	clear_list(0);
	uicontext->read_entry=1;
}

void show_all_slots(ui_context * uicontext,int drive)
{
	char tmp_str[81];
	disk_in_drive_v2 * drive_slots_ptr;
	unsigned short i,xoffset,slotnumber;

	#ifdef DEBUG
	dbg_printf("show_all_slots : %d (nb of drive : %d)\n",drive,uicontext->number_of_drive);
	#endif

	clear_list(0);

	if( drive >= uicontext->number_of_drive )
		return;

	hxc_printf(CENTER_ALIGNED,0,FILELIST_Y_POS,"--- Drive %c slots selection ---",'A'+drive);

	for ( i = 1; i < NUMBER_OF_FILE_ON_DISPLAY; i++ )
	{
		slotnumber = i + (uicontext->slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1));

		if( slotnumber < uicontext->config_file_number_max_of_slot)
		{
			memset(tmp_str,0,sizeof(tmp_str));
			drive_slots_ptr = &disks_slots[ (slotnumber*uicontext->number_of_drive) + drive];
			if( drive_slots_ptr->size )
			{
				memcpy(tmp_str,&drive_slots_ptr->name,MAX_SHORT_NAME_LENGHT);
			}

			if( slotnumber > 99 )
				xoffset = 0;
			else
			{
				if( slotnumber > 9 )
				{
					xoffset = 8;
					hxc_printf(LEFT_ALIGNED,0,(uint16_t)(FILELIST_Y_POS + (i*8)),"0");
				}
				else
				{
					xoffset = 16;
					hxc_printf(LEFT_ALIGNED,0,(uint16_t)(FILELIST_Y_POS + (i*8)),"00");
				}
			}

			hxc_printf(LEFT_ALIGNED,xoffset,(uint16_t)(FILELIST_Y_POS + (i*8)),"%d:%s", slotnumber, tmp_str);
		}
	}
}

int getext(char * path,char * exttodest)
{
	int i;

	i = 0;
	while(path[i] && i<256)
	{
		i++;
	}

	while( i && path[i]!='.')
	{
		i--;
	}

	if(path[i]=='.')
	{
		i++;
		exttodest[0] = path[i];
		exttodest[1] = path[i+1];
		exttodest[2] = path[i+2];

		// Remove trailing space
		i = 2;
		while(i>=0 && exttodest[i]==' ')
		{
			exttodest[i] = 0;
			i--;
		}
	}


	return 0;
}

void print_help()
{
	int i;
	int key;

	i = 0;

	key = FCT_NO_FUNCTION;

	while(help_pages[i].txt && key!=FCT_ESCAPE)
	{
		clear_list(0);

		hxc_print(help_pages[i].align,0,HELP_Y_POS, (char*)help_pages[i].txt);

		do
		{
			key = wait_function_key();
		}while(key!=FCT_SELECT_FILE_DRIVEA && key!=FCT_ESCAPE);

		i++;
	}
}

void ui_savereboot(ui_context * uicontext,int preselected_slot)
{
	hxc_printf_box((char*)save_and_restart_msg);
	save_cfg_file(uicontext,cfgfile_header,preselected_slot);
	restore_box();
	hxc_printf_box((char*)reboot_msg);
	sleep(1);
	jumptotrack(0);
	reboot();
}

void ui_save(ui_context * uicontext)
{
	hxc_printf_box((char*)save_msg);
	save_cfg_file(uicontext,cfgfile_header,-1);
	restore_box();
}

void ui_reboot(ui_context * uicontext)
{
	hxc_printf_box((char*)reboot_msg);
	sleep(1);
	jumptotrack(0);
	reboot();
}

void ui_config_menu(ui_context * uicontext)
{
	int i;
	unsigned char c;
	cfgfile * cfgfile_ptr;

	clear_list(0);
	cfgfile_ptr=(cfgfile * )cfgfile_header;

	i=0;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "HxC Floppy Emulator settings:");

	i=2;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "Track step sound :");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "User interface sound:");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d   ",cfgfile_ptr->buzzer_duty_cycle);

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "LCD Backlight standby:");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s",cfgfile_ptr->back_light_tmr);

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "SD/USB Standby:");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s",cfgfile_ptr->standby_tmr);

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "DF1 drive :");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "Load AUTOBOOT.HFE at power up :");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->startup_mode & START_MODE_SLOT_0?"on":"off");

	i++;
	hxc_print(LEFT_ALIGNED,0,HELP_Y_POS+(i*8), "Eject disk at power up :");
	hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->startup_mode & START_MODE_DSKEJECTED?"on":"off");

	i=i+2;
	hxc_print(CENTER_ALIGNED,0,HELP_Y_POS+(i*8), "--- Exit ---");

	i=2;
	invert_line(0,HELP_Y_POS+(i*8));
	do
	{
		c=wait_function_key();
		switch(c)
		{
			case FCT_UP_KEY:
				invert_line(0,HELP_Y_POS+(i*8));
				if(i>2) i--;
				invert_line(0,HELP_Y_POS+(i*8));
			break;
			case FCT_DOWN_KEY:
				invert_line(0,HELP_Y_POS+(i*8));
				if(i<10) i++;
				invert_line(0,HELP_Y_POS+(i*8));
			break;

			case FCT_LEFT_KEY:
			case FCT_RIGHT_KEY:
				invert_line(0,HELP_Y_POS+(i*8));
				switch(i)
				{
					case 2:
						cfgfile_ptr->step_sound =~ cfgfile_ptr->step_sound;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");
					break;
					case 3:
						if(	c == FCT_LEFT_KEY )
						{
							if(cfgfile_ptr->buzzer_duty_cycle)
								cfgfile_ptr->buzzer_duty_cycle--;
						}
						else
						{
							if(cfgfile_ptr->buzzer_duty_cycle<0x80)
								cfgfile_ptr->buzzer_duty_cycle++;
							cfgfile_ptr->ihm_sound = 0xFF;
						}
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d  ",cfgfile_ptr->buzzer_duty_cycle);
						if(!cfgfile_ptr->buzzer_duty_cycle) cfgfile_ptr->ihm_sound=0x00;
					break;
					case 4:
						if(	c == FCT_LEFT_KEY )
						{
							if(cfgfile_ptr->back_light_tmr)
								cfgfile_ptr->back_light_tmr--;
						}
						else
						{
							if(cfgfile_ptr->back_light_tmr<0xFF)
								cfgfile_ptr->back_light_tmr++;
						}

						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->back_light_tmr);
					break;

					case 5:
						if(	c == FCT_LEFT_KEY )
						{
							if(cfgfile_ptr->standby_tmr)
								cfgfile_ptr->standby_tmr--;
						}
						else
						{
							if(cfgfile_ptr->standby_tmr<0xFF)
								cfgfile_ptr->standby_tmr++;
						}

						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->standby_tmr);
					break;

					case 6:
						cfgfile_ptr->enable_drive_b=~cfgfile_ptr->enable_drive_b;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");
					break;

					case 7:
						cfgfile_ptr->startup_mode ^= START_MODE_SLOT_0;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode & START_MODE_SLOT_0)?"on":"off");
					break;

					case 8:
						cfgfile_ptr->startup_mode ^= START_MODE_DSKEJECTED;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode & START_MODE_DSKEJECTED)?"on":"off");
					break;
				}
				invert_line(0,HELP_Y_POS+(i*8));
			break;

			case FCT_SELECT_FILE_DRIVEA:
				invert_line(0,HELP_Y_POS+(i*8));
				switch(i)
				{
					case 2:
						cfgfile_ptr->step_sound=~cfgfile_ptr->step_sound;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");
					break;
					case 6:
						cfgfile_ptr->enable_drive_b=~cfgfile_ptr->enable_drive_b;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");
					break;
					case 7:
						cfgfile_ptr->startup_mode ^= START_MODE_SLOT_0;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode & START_MODE_SLOT_0)?"on":"off");
					break;
					case 8:
						cfgfile_ptr->startup_mode ^= START_MODE_DSKEJECTED;
						hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode & START_MODE_DSKEJECTED)?"on":"off");
					break;
				}

				invert_line(0,HELP_Y_POS+(i*8));
			break;
		}
	}while( (c!=FCT_SELECT_FILE_DRIVEA) || i != 10 );

	memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));
	clear_list(0);
	uicontext->read_entry=1;
}

int ui_command_menu(ui_context * uicontext)
{
	unsigned char key;
	////////////////////
	// command menu

	clear_list(0);
	hxc_printf(CENTER_ALIGNED,0,FILELIST_Y_POS,(char*)command_menu_msg);
	invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));

	do
	{
		key=wait_function_key();

		switch(key)
		{
			case FCT_UP_KEY: // UP
				//invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
				if(uicontext->slotselectorpos)
				{
					uicontext->slotselectorpos--;
					invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos+1)*8));
					invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos)*8));
				}
			break;
			case FCT_DOWN_KEY: // Down
				if(uicontext->slotselectorpos<(NUMBER_OF_FILE_ON_DISPLAY-1))
				{
					uicontext->slotselectorpos++;
					invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos-1)*8));
					invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
				}
			break;

			case FCT_RIGHT_KEY: // Right

			break;

			case FCT_LEFT_KEY:

			break;

			case FCT_SELECT_FILE_DRIVEA:
				if(!uicontext->slotselectorpos)
				{
					uicontext->page_mode_index++;

					if(uicontext->page_mode_index >= (int)(2 + uicontext->number_of_drive))
						uicontext->page_mode_index = 0;
				}

			break;
			case FCT_HELP:
				print_help();
				clear_list(0);
				hxc_printf(CENTER_ALIGNED,0,FILELIST_Y_POS,(char*)command_menu_msg);
				invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
			break;

			case FCT_SAVEREBOOT:
				ui_savereboot(uicontext,-1);
			break;

			case FCT_SAVE:
				ui_save(uicontext);
			break;

			case FCT_REBOOT:
				ui_reboot(uicontext);
			break;
		}
	}while(key != FCT_SELECT_FILE_DRIVEA && key != FCT_ESCAPE );

	if( key == FCT_SELECT_FILE_DRIVEA )
	{
		switch(uicontext->slotselectorpos)
		{
			case 0:
				clear_list(0);
				return 0;
			break;

			case 2:
				ui_savereboot(uicontext,-1);
			break;

			case 3:
				ui_save(uicontext);
			break;

			case 4:
				ui_reboot(uicontext);
			break;

			case 6:
				uicontext->colormode++;
				set_color_scheme(uicontext->colormode);
				cfgfile_header[256+128] = uicontext->colormode;
				waitms(100);
				return 0;
			break;
			case 7:
				ui_config_menu(uicontext);
				return 0;
			break;

			case 9:
				print_help();
				return 0;
			break;

			default:
				clear_list(0);
				return 0;
			break;
		}
	}

	clear_list(0);

	return 1;
}

int ui_slots_menu(ui_context * uicontext)
{
	unsigned char key;
	int slot,i;

	#ifdef DEBUG
	dbg_printf("enter ui_slots_menu\n");
	#endif

	////////////////////
	// Slots list menu

	show_all_slots(uicontext,uicontext->page_mode_index);

	invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));

	do
	{
		key=wait_function_key();

		switch(key)
		{
			case FCT_UP_KEY: // UP
				if(uicontext->slotselectorpos)
				{
					uicontext->slotselectorpos--;
					invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos+1)*8));
					invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos)*8));
				}
				else
				{
					if(uicontext->slotselectorpage)
					{
						uicontext->slotselectorpos = (NUMBER_OF_FILE_ON_DISPLAY-1);
						uicontext->slotselectorpage--;
					}

					show_all_slots(uicontext,uicontext->page_mode_index);
					invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
				}
			break;
			case FCT_DOWN_KEY: // Down

				if(uicontext->slotselectorpos + (uicontext->slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)) < uicontext->config_file_number_max_of_slot )
				{
					uicontext->slotselectorpos++;
					if(uicontext->slotselectorpos>(NUMBER_OF_FILE_ON_DISPLAY-1))
					{
						uicontext->slotselectorpos = 1;
						uicontext->slotselectorpage++;

						show_all_slots(uicontext,uicontext->page_mode_index);
						invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
					}
					else
					{
						invert_line(0,FILELIST_Y_POS+((uicontext->slotselectorpos-1)*8));
						invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
					}
				}
			break;

			case FCT_RIGHT_KEY: // Right
				if(uicontext->slotselectorpos + ((uicontext->slotselectorpage+1) * (NUMBER_OF_FILE_ON_DISPLAY-1)) < uicontext->config_file_number_max_of_slot )
				{
					uicontext->slotselectorpage++;
				}

				show_all_slots(uicontext,uicontext->page_mode_index);
				invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
			break;

			case FCT_LEFT_KEY:
				if(uicontext->slotselectorpage)
					uicontext->slotselectorpage--;

				show_all_slots(uicontext,uicontext->page_mode_index);
				invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
			break;

			case FCT_CLEARSLOT:
				slot = (uicontext->slotselectorpos + (uicontext->slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)));

				memset((void*)&disks_slots[(slot*uicontext->number_of_drive) + uicontext->page_mode_index ],0,sizeof(disk_in_drive_v2));

				i = 0;
				while( i < uicontext->number_of_drive && !disks_slots[(slot*uicontext->number_of_drive)+i].type[0] )
				{
					i++;
				}

				uicontext->change_map[slot>>3] |= (0x80 >> (slot&7));

				// All drive empty - clear the slot
				if( i == uicontext->number_of_drive )
				{
					uicontext->slot_map[slot>>3] &= ~(0x80 >> (slot&7));
				}

				show_all_slots(uicontext,uicontext->page_mode_index);
				invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
			break;

			case FCT_SELECTSAVEREBOOT:

				slot = (uicontext->slotselectorpos + (uicontext->slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)));
				if( uicontext->slot_map[slot>>3] & (0x80 >> (slot&7)) )
				{
					ui_savereboot(uicontext,slot);
				}

			break;
			case FCT_SELECT_FILE_DRIVEA:
				if(!uicontext->slotselectorpos)
				{
					uicontext->page_mode_index++;

					if(uicontext->page_mode_index >= 2 + uicontext->number_of_drive)
						uicontext->page_mode_index = 0;
				}
			break;

			case FCT_HELP:
				print_help();

				show_all_slots(uicontext,uicontext->page_mode_index);
				invert_line(0,FILELIST_Y_POS+(uicontext->slotselectorpos*8));
			break;

			case FCT_SAVEREBOOT:
				ui_savereboot(uicontext,-1);
			break;

			case FCT_SAVE:
				ui_save(uicontext);
			break;

			case FCT_REBOOT:
				ui_reboot(uicontext);
			break;
		}
	}while(key != FCT_SELECT_FILE_DRIVEA && key != FCT_ESCAPE );

	if( key == FCT_SELECT_FILE_DRIVEA && !uicontext->slotselectorpos )
	{
		clear_list(0);

		#ifdef DEBUG
		dbg_printf("leave ui_slots_menu\n");
		#endif
		return 0;
	}

	if( ( key != FCT_ESCAPE ) && ( uicontext->page_mode_index < uicontext->number_of_drive ) )
	{
		slot = (uicontext->slotselectorpos + (uicontext->slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)));

		#ifdef DEBUG
		dbg_printf("set slot %d (page %d, pos %d)\n",slot,uicontext->slotselectorpage,uicontext->slotselectorpos);
		#endif

		memcpy( (void*)&disks_slots[ (slot*uicontext->number_of_drive) + ( uicontext->page_mode_index ) ],
				(void*)&DirectoryEntry_tab[ uicontext->selectorpos ],
				sizeof(disk_in_drive_v2)
				);

		uicontext->slot_map[slot>>3] |= (0x80 >> (slot&7));
		uicontext->change_map[slot>>3] |= (0x80 >> (slot&7));
	}

	clear_list(0);

	#ifdef DEBUG
	dbg_printf("leave ui_slots_menu\n");
	#endif
	return 1;
}

void ui_mainfileselector(ui_context * uicontext)
{
	int ret;
	short i,j,y_pos;
	unsigned char displayentry;
	unsigned char entrytype_icon;
	unsigned char key,c;
	unsigned char last_file;
	disk_in_drive_v2_long * disk_ptr;

	y_pos=FILELIST_Y_POS;

	#ifdef DEBUG
	dbg_printf("enter ui_mainfileselector\n");
	#endif

	uicontext->page_mode_index = ( 2 + uicontext->number_of_drive ) - 1;

	#ifdef DEBUG
	dbg_printf("page_mode_index : %d\n",uicontext->page_mode_index);
	#endif

	clear_list(0);
	for(;;)
	{
		#ifdef DEBUG
		dbg_printf("Page : %d Selector pos : %d\n",uicontext->page_number,uicontext->selectorpos);
		#endif

		i=0;
		do
		{
			memset(&DirectoryEntry_tab[i],0,sizeof(disk_in_drive_v2_long));
			i++;
		}while((i<NUMBER_OF_FILE_ON_DISPLAY));

		last_file=0x00;

		y_pos = FILELIST_Y_POS;
		hxc_printf(CENTER_ALIGNED,0,y_pos,"--- SD/USB Media files ---");
		y_pos += 8;
		i = 1;
		do
		{
			displayentry=0xFF;
			if(!fl_readdir(&file_list_status, &dir_entry))
			{
				if(uicontext->filtermode)
				{
					strlwr(dir_entry.filename);

					if(!strstr(dir_entry.filename,uicontext->filter))
					{
						displayentry=0x00;
					}
				}

				if(displayentry)
				{
					if(dir_entry.is_dir)
					{
						entrytype_icon = 10;
						DirectoryEntry_tab[i].attributes=0x10;
					}
					else
					{
						entrytype_icon = 12;
						DirectoryEntry_tab[i].attributes=0x00;
					}

					hxc_printf(LEFT_ALIGNED | DONTPARSE,0,y_pos," %c%s",entrytype_icon,dir_entry.filename);

					y_pos=y_pos+8;

					// Get the file name extension.
					getext(dir_entry.filename,(char*)&DirectoryEntry_tab[i].type);

					j = 0;
					while(j<MAX_LONG_NAME_LENGHT && dir_entry.filename[j])
					{
						DirectoryEntry_tab[i].name[j] = dir_entry.filename[j];
						j++;
					}

					DirectoryEntry_tab[i].name[j] = 0x00;

					DirectoryEntry_tab[i].firstCluster = ENDIAN_32BIT(dir_entry.cluster) ;
					DirectoryEntry_tab[i].size =  ENDIAN_32BIT(dir_entry.size);

					#ifdef DEBUG
					dbg_printf("Entry : %s Size:%d Cluster 0x%.8X\n",DirectoryEntry_tab[i].name,DirectoryEntry_tab[i].size,DirectoryEntry_tab[i].firstCluster);
					#endif

					i++;
				}
			}
			else
			{
				last_file=0xFF;
				i=NUMBER_OF_FILE_ON_DISPLAY;
			}

		}while( i < NUMBER_OF_FILE_ON_DISPLAY );

		uicontext->filtermode=0;

		if(uicontext->page_number < MAX_PAGES_PER_DIRECTORY - 1 )
			memcpy(&file_list_status_tab[uicontext->page_number+1],&file_list_status ,sizeof(FL_DIR));

		hxc_print(LEFT_ALIGNED,0,FILELIST_Y_POS+(uicontext->selectorpos*8),">");
		invert_line(0,FILELIST_Y_POS+(uicontext->selectorpos*8));

		uicontext->read_entry=0;

		do
		{
			key=wait_function_key();
			if(1)
			{
				switch(key)
				{
				case FCT_UP_KEY: // UP
					invert_line(0,FILELIST_Y_POS+(uicontext->selectorpos*8));
					hxc_print(LEFT_ALIGNED,0,FILELIST_Y_POS+(uicontext->selectorpos*8)," ");

					uicontext->selectorpos--;
					if(uicontext->selectorpos<0)
					{
						uicontext->selectorpos = NUMBER_OF_FILE_ON_DISPLAY-1;
						if(uicontext->page_number)
							uicontext->page_number--;
						clear_list(0);
						uicontext->read_entry=1;
						memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));

						#ifdef DEBUG
						dbg_printf("Page change : %d\n",uicontext->page_number);
						#endif
					}
					else
					{
						hxc_print(LEFT_ALIGNED,0,FILELIST_Y_POS+(uicontext->selectorpos*8),">");
						invert_line(0,FILELIST_Y_POS+(uicontext->selectorpos*8));
					}
					break;

				case FCT_DOWN_KEY: // Down
					invert_line(0,FILELIST_Y_POS+(uicontext->selectorpos*8));
					hxc_print(LEFT_ALIGNED,0,FILELIST_Y_POS+(uicontext->selectorpos*8)," ");

					uicontext->selectorpos++;
					if(uicontext->selectorpos>=NUMBER_OF_FILE_ON_DISPLAY)
					{
						uicontext->selectorpos = 1;
						clear_list(0);
						uicontext->read_entry=1;
						if(!last_file && uicontext->page_number < MAX_PAGES_PER_DIRECTORY)
							uicontext->page_number++;
						memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));

						#ifdef DEBUG
						dbg_printf("Page change : %d\n",uicontext->page_number);
						#endif
					}
					else
					{
						hxc_print(LEFT_ALIGNED,0,FILELIST_Y_POS+(uicontext->selectorpos*8),">");
						invert_line(0,FILELIST_Y_POS+(uicontext->selectorpos*8));
					}

					break;

				case FCT_RIGHT_KEY: // Right
					if(!last_file && uicontext->page_number < MAX_PAGES_PER_DIRECTORY)
						uicontext->page_number++;

					memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));

					clear_list(0);
					uicontext->read_entry=1;

					#ifdef DEBUG
					dbg_printf("Page change : %d\n",uicontext->page_number);
					#endif
					break;

				case FCT_LEFT_KEY:
					if(uicontext->page_number)
						uicontext->page_number--;

					memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));

					clear_list(0);
					uicontext->read_entry=1;

					#ifdef DEBUG
					dbg_printf("Page change : %d\n",uicontext->page_number);
					#endif
					break;

				case FCT_SELECT_FILE_DRIVEA:
				case FCT_SHOWSLOTS:
					disk_ptr=(disk_in_drive_v2_long * )&DirectoryEntry_tab[uicontext->selectorpos];

					if( disk_ptr->attributes & 0x10 )
					{
						enter_sub_dir(uicontext,disk_ptr);
					}
					else
					{
						if(!uicontext->selectorpos || uicontext->page_mode_index>= 1 + uicontext->number_of_drive)
							uicontext->page_mode_index = 0;

						if(!uicontext->selectorpos)
							uicontext->slotselectorpos = 0;
						do
						{
							if(uicontext->page_mode_index < uicontext->number_of_drive )
							{
								ret = ui_slots_menu(uicontext);
							}
							else
							{
								ret = ui_command_menu(uicontext);
							}
						}while(!ret && uicontext->page_mode_index <  uicontext->number_of_drive + 1 );

						memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));
						uicontext->read_entry = 1;
					}
					break;

				case FCT_SELECTSAVEREBOOT:
					disk_ptr=(disk_in_drive_v2_long * )&DirectoryEntry_tab[uicontext->selectorpos];

					if( disk_ptr->attributes & 0x10 )
					{
						enter_sub_dir(uicontext,disk_ptr);
					}
					else
					{
						// Update the Slot 1, select it and reboot...
						memcpy((void*)&disks_slots[1*uicontext->number_of_drive],(void*)&DirectoryEntry_tab[uicontext->selectorpos],sizeof(disk_in_drive_v2));
						uicontext->slot_map[1>>3] |= (0x80 >> (1&7));
						uicontext->change_map[1>>3] |= (0x80 >> (1&7));
						ui_savereboot(uicontext,1);
					}
					break;

				case FCT_SAVEREBOOT:
					ui_savereboot(uicontext,-1);
					break;

				case FCT_SAVE:
					ui_save(uicontext);
					memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));
					clear_list(0);
					uicontext->read_entry=1;
					break;

				case FCT_REBOOT:
					ui_reboot(uicontext);
					break;

				case FCT_HELP:
					print_help();
					memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));
					clear_list(0);
					uicontext->read_entry=1;
					break;

				case FCT_EMUCFG:
					ui_config_menu(uicontext);
					break;

				case FCT_CHGCOLOR:
					uicontext->colormode++;
					set_color_scheme(uicontext->colormode);
					cfgfile_header[256+128]=uicontext->colormode;
					waitms(100);
					break;

				case FCT_TOP:
					uicontext->page_number=0;
					memcpy(&file_list_status ,&file_list_status_tab[uicontext->page_number],sizeof(FL_DIR));
					clear_list(0);
					uicontext->read_entry=1;
					break;

				case FCT_SEARCH:
					uicontext->filtermode=0xFF;
					hxc_print(LEFT_ALIGNED,(SCREEN_XRESOL/2)+8*8,CURDIR_Y_POS,"Search:                     ");
					flush_char();
					i=0;
					do
					{
						uicontext->filter[i]=0;
						c=get_char();
						if(c!='\n')
						{
							uicontext->filter[i]=c;
							hxc_printf(LEFT_ALIGNED,(SCREEN_XRESOL/2)+(8*8)+(8*8)+(8*i),CURDIR_Y_POS,"%c",c);
						}
						i++;
					}while(c!='\n' && i<16);
					uicontext->filter[i]=0;

					strlwr(uicontext->filter);
					hxc_printf(LEFT_ALIGNED,SCREEN_XRESOL/2+(8*8)+(8*8),CURDIR_Y_POS,"[%s]",uicontext->filter);
					uicontext->selectorpos=0;
					uicontext->page_number=0;
					memcpy(&file_list_status ,&file_list_status_tab[0],sizeof(FL_DIR));

					clear_list(0);
					uicontext->read_entry=1;
					break;

				default:
					//printf("err %d!\n",key);
					break;
				}
			}
		}while(!uicontext->read_entry);
	}
}

int main(int argc, char* argv[])
{
	unsigned short i;
	int bootdev;
	ui_context * uicontext;

	uicontext = &g_ui_ctx;

	if(process_command_line(argc, argv))
	{
		return 0;
	}

	cfg_file_handle = 0;
	memset( uicontext,0,sizeof(ui_context));
	strcpy( uicontext->currentPath, "/" );

	strcpy(FIRMWAREVERSION,"-------");

	init_display();

	init_display_buffer();

	#ifdef DEBUG
	dbg_printf("Init display Done\n");
	#endif

	io_floppy_timeout = 0;

	if(argv)
		bootdev = get_start_unit(argv[0]);
	else
		bootdev = get_start_unit(0);

	if( bootdev < 0 )
	{
		hxc_printf_box("ERROR: HxC Drive not detected !");
		lockup();
	}

	init_fdc(bootdev);

	#ifdef DEBUG
	dbg_printf("init_fdc Done\n");
	#endif

	if(media_init())
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
			hxc_printf_box("ERROR: Media attach failed !");
			lockup();
		}

		#ifdef DEBUG
		dbg_printf("fl_attach_media done\n");
		#endif

		hxc_printf_box("Reading HXCSDFE.CFG ...");

		cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
		if (!cfg_file_handle)
		{
			hxc_printf_box("ERROR: Can't open HXCSDFE.CFG !");
			lockup();
		}

		read_cfg_file(uicontext,cfgfile_header);

		#ifdef DEBUG
		dbg_printf("read_cfg_file done\n");
		#endif

		if(cfgfile_header[256+128]!=0xFF)
			set_color_scheme(cfgfile_header[256+128]);

		displayFolder(uicontext);

		fl_opendir(uicontext->currentPath, &file_list_status);

		for(i=0;i<MAX_PAGES_PER_DIRECTORY;i++)
		{
			memcpy(&file_list_status_tab[i],&file_list_status ,sizeof(FL_DIR));
		}

		ui_mainfileselector(uicontext);
	}
	lockup();
	return 0;
}
