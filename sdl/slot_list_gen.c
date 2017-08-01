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

// Text file Slots list import/export

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "hardware.h"

#include "cfg_file.h"

#include "conf.h"
#include "ui_context.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

extern int media_init();
extern int media_read(uint32 sector, uint8 *buffer, uint32 sector_count);
extern int media_write(uint32 sector, uint8 *buffer, uint32 sector_count);
extern FL_FILE * cfg_file_handle;
extern void init_fdc(int drive);
extern int getext(char * path,char * exttodest);
extern char read_cfg_file(ui_context * uicontext,unsigned char * cfgfile_header);
extern char save_cfg_file(ui_context * uicontext,unsigned char * sdfecfg_file, int pre_selected_slot);


extern ui_context g_ui_ctx;
extern disk_in_drive_v2 disks_slots[MAX_NUMBER_OF_SLOT];
char scanfolderpath[1024];


int attach_and_init_media()
{
	if(media_init())
	{
		// Initialise File IO Library
		fl_init();

		/* Attach media access functions to library*/
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
		{
			printf("ERROR: Media attach failed !");
			return 0;
		}

		return 1;
	}
	return 0;
}

int get_path_from_cluster(char * path, unsigned long cluster)
{
	struct fs_dir_ent dir_entry;
	FL_DIR file_list_status;
	int pathlen;

	fl_opendir(scanfolderpath, &file_list_status);
	while(!fl_readdir(&file_list_status, &dir_entry))
	{
		if(dir_entry.is_dir)
		{
			#ifdef DEBUG
			printf("Folder Entry : %s Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.cluster));
			#endif

			if(strcmp(dir_entry.filename,".") && strcmp(dir_entry.filename,".."))
			{
				pathlen = strlen(scanfolderpath);
				strcat(scanfolderpath,"/");
				strcat(scanfolderpath,dir_entry.filename);
				if( get_path_from_cluster( path, cluster) )
					return 1;
				scanfolderpath[pathlen] = 0;
			}
		}
		else
		{
			#ifdef DEBUG
			printf("File Entry : %s Size:%d Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.size),ENDIAN_32BIT(dir_entry.cluster));
			#endif

			if(cluster == ENDIAN_32BIT(dir_entry.cluster) )
			{
				#ifdef DEBUG
				printf("Found File Entry : %s/%s Size:%d Cluster 0x%.8X\n",scanfolderpath,dir_entry.filename,ENDIAN_32BIT(dir_entry.size),ENDIAN_32BIT(dir_entry.cluster));
				#endif

				sprintf(path,"%s/%s",scanfolderpath,dir_entry.filename);
				return 1;
			}
		}
	}
	return 0;
}

int clear_slots(ui_context * uicontext)
{
	disk_in_drive_v2 * drive_slots_ptr;
	int slotnumber;
	int drive;

	drive = 0;

	printf("Clear all slots...\n");

	for ( slotnumber = 1; slotnumber < uicontext->config_file_number_max_of_slot; slotnumber++ )
	{
		for(drive = 0;drive < uicontext->number_of_drive;drive++)
		{
			drive_slots_ptr = &disks_slots[ (slotnumber*uicontext->number_of_drive) + drive];

			memset(drive_slots_ptr,0,sizeof(disk_in_drive_v2));
			uicontext->slot_map[slotnumber>>3] &= ~(0x80 >> (slotnumber&7));
			uicontext->change_map[slotnumber>>3] |= (0x80 >> (slotnumber&7));
		}
	}

	return 0;
}

int get_slot_from_cluster(ui_context * uicontext,unsigned long cluster)
{
	disk_in_drive_v2 * drive_slots_ptr;
	int slotnumber;
	int drive;

	drive = 0;

	for ( slotnumber = 1; slotnumber < uicontext->config_file_number_max_of_slot; slotnumber++ )
	{
		for(drive = 0; drive < uicontext->number_of_drive; drive++)
		{
			drive_slots_ptr = &disks_slots[ (slotnumber*uicontext->number_of_drive) + drive];

			if( (uicontext->slot_map[slotnumber>>3] & (0x80 >> (slotnumber&7))) )
			{
				if( cluster == drive_slots_ptr->firstCluster )
				{
					return slotnumber;
				}
			}
		}
	}

	return -1;
}

int scan_tree_slot(ui_context * uicontext,FILE * fout)
{
	struct fs_dir_ent dir_entry;
	FL_DIR file_list_status;
	int pathlen,slot;

	fl_opendir(scanfolderpath, &file_list_status);
	while(!fl_readdir(&file_list_status, &dir_entry))
	{
		if(dir_entry.is_dir)
		{
			#ifdef DEBUG
			printf("Folder Entry : %s Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.cluster));
			#endif

			if(strcmp(dir_entry.filename,".") && strcmp(dir_entry.filename,".."))
			{
				pathlen = strlen(scanfolderpath);
				strcat(scanfolderpath,"/");
				strcat(scanfolderpath,dir_entry.filename);
				if( scan_tree_slot( uicontext, fout ) )
					return 1;
				scanfolderpath[pathlen] = 0;
			}
		}
		else
		{
			#ifdef DEBUG
			printf("File Entry : %s Size:%d Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.size),ENDIAN_32BIT(dir_entry.cluster));
			#endif

			slot = get_slot_from_cluster(uicontext,ENDIAN_32BIT(dir_entry.cluster));

			if( slot >= 0 )
			{
				// Mark it as found... (good)
				uicontext->change_map[slot>>3] |= (0x80 >> (slot&7));

				// If found...
				if(fout)
					fprintf(fout,"%.5d:%s/%s\r\n",slot,scanfolderpath,dir_entry.filename);

				printf("Slot %d:%s/%s\n",slot,scanfolderpath,dir_entry.filename);
			}
		}
	}
	return 0;
}

int check_slots(ui_context * uicontext,char * outputfile,int fix)
{
	char tmp_str[81];
	disk_in_drive_v2 * drive_slots_ptr;
	int slotnumber;
	int drive,errorcnt;
	FILE * fout;

	drive = 0;
	errorcnt = 0;
	fout = 0;
	if(outputfile)
	{
		if(strlen(outputfile))
		{
			fout = fopen(outputfile,"wb");
			if(!fout)
			{
				printf("Error ! can't create %s !\n",outputfile);
			}
		}
	}

	printf("Now checking and listing all slots...\n");

	scan_tree_slot(uicontext,fout);

	// Check that all slot have been found
	for ( slotnumber = 1; slotnumber < uicontext->config_file_number_max_of_slot; slotnumber++ )
	{
		if( (uicontext->slot_map[slotnumber>>3] & (0x80 >> (slotnumber&7))) )
		{
			if( !(uicontext->change_map[slotnumber>>3] & (0x80 >> (slotnumber&7))) )
			{
				// Bad Slot !
				memset(tmp_str,0,sizeof(tmp_str));
				drive_slots_ptr = &disks_slots[ (slotnumber*uicontext->number_of_drive) + drive];
				memcpy(tmp_str,&drive_slots_ptr->name,MAX_SHORT_NAME_LENGHT);

				printf("Slot %d:%s -> ERROR ! Not found ! Bad Slot Entry !\n",slotnumber,tmp_str);
				errorcnt++;
				if(fix)
				{
					printf("Clear Slot %d\n",slotnumber);
					memset(drive_slots_ptr,0,sizeof(disk_in_drive_v2));
					uicontext->slot_map[slotnumber>>3] &= ~(0x80 >> (slotnumber&7));
					uicontext->change_map[slotnumber>>3] |= (0x80 >> (slotnumber&7));
				}
			}
			else
			{
				uicontext->change_map[slotnumber>>3] &= ~(0x80 >> (slotnumber&7));
			}
		}
	}

	if(fout)
		fclose(fout);

	return errorcnt;
}

int generate_slot_list(char * outputfile,int fix)
{
	ui_context * uicontext;
	unsigned char cfgfile_header[512];
	int errorcnt;

	uicontext = &g_ui_ctx;

	memset( uicontext,0,sizeof(ui_context));
	strcpy( uicontext->currentPath, "/" );

	init_fdc(0);

	attach_and_init_media();

	printf("Reading HXCSDFE.CFG ...\n");

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (!cfg_file_handle)
	{
		printf("ERROR: Can't open HXCSDFE.CFG !");
		return 0;
	}

	read_cfg_file(uicontext,(unsigned char*)&cfgfile_header);

	errorcnt = check_slots(uicontext,outputfile,fix);

	printf("%d Bad slot(s) found...\n",errorcnt);

	if(errorcnt && fix)
	{
		printf("Saving...\n");
		save_cfg_file(uicontext,(unsigned char*)&cfgfile_header, -1);
	}

	scanfolderpath[0] = 0;

	return 0;
}

int insert_slot_list(char * inputfile)
{
	ui_context * uicontext;
	unsigned char cfgfile_header[512];
	char linebuffer[1024];
	char * filename;
	char * ptr;
	FILE * fin;
	int j,drive;
	FL_FILE * slotfile;
	int slotnumber;
	disk_in_drive_v2_long DirectoryEntry;

	drive = 0;

	uicontext = &g_ui_ctx;

	memset( uicontext,0,sizeof(ui_context));
	strcpy( uicontext->currentPath, "/" );

	init_fdc(0);

	attach_and_init_media();

	printf("Reading HXCSDFE.CFG ...\n");

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (!cfg_file_handle)
	{
		printf("ERROR: Can't open HXCSDFE.CFG !");
		return 0;
	}

	read_cfg_file(uicontext,(unsigned char*)&cfgfile_header);

	printf("Opening %s...\n",inputfile);
	fin = fopen(inputfile,"r");
	if(fin)
	{
		while(!feof(fin))
		{
			memset(&DirectoryEntry,0,sizeof(DirectoryEntry));
			if(fgets(linebuffer, sizeof(linebuffer), fin))
			{
				filename = strchr ( linebuffer, ':' );
				if(filename)
				{
					*filename = 0;
					filename++;

					ptr = strchr ( filename, '\n' );
					if(ptr)
						*ptr = 0;

					ptr = strchr ( filename, '\r' );
					if(ptr)
						*ptr = 0;

					slotfile = fl_fopen(filename, "r");
					if (!slotfile)
					{
						printf("ERROR: Can't open %s !\n",filename);
					}
					else
					{
						slotnumber = atoi(linebuffer);
						if(slotnumber < uicontext->config_file_number_max_of_slot )
						{
							printf("Slot %d : Set to %s\n",slotnumber,filename);

							DirectoryEntry.attributes = slotfile->attributes;

							// Get the file name extension.
							getext(slotfile->filename,(char*)&DirectoryEntry.type);

							j = 0;
							while(j<MAX_LONG_NAME_LENGHT && slotfile->filename[j])
							{
								DirectoryEntry.name[j] = slotfile->filename[j];
								j++;
							}

							DirectoryEntry.name[j] = 0x00;

							DirectoryEntry.firstCluster = ENDIAN_32BIT(slotfile->startcluster) ;
							DirectoryEntry.size = ENDIAN_32BIT(slotfile->filelength);

							#ifdef DEBUG
							printf("Entry : %s Size:%d Cluster 0x%.8X\n",DirectoryEntry.name,DirectoryEntry.size,DirectoryEntry.firstCluster);
							#endif

							memcpy( (void*)&disks_slots[ (slotnumber * uicontext->number_of_drive) + drive ],
								(void*)&DirectoryEntry,
								sizeof(disk_in_drive_v2)
								);

							uicontext->slot_map[slotnumber>>3] |= (0x80 >> (slotnumber&7));
							uicontext->change_map[slotnumber>>3] |= (0x80 >> (slotnumber&7));
						}
						else
						{
							printf("Error : Can't insert slot %d - Space not available into the cfg file !\n",slotnumber);
						}

						fl_fclose(slotfile);
					}
				}
			}
		}

		fclose(fin);

		save_cfg_file(uicontext,(unsigned char*)&cfgfile_header, -1);
	}

	return 0;
}

int clear_all_slots()
{
	ui_context * uicontext;
	unsigned char cfgfile_header[512];
	uicontext = &g_ui_ctx;

	memset( uicontext,0,sizeof(ui_context));
	strcpy( uicontext->currentPath, "/" );

	init_fdc(0);

	attach_and_init_media();

	printf("Reading HXCSDFE.CFG ...\n");

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (!cfg_file_handle)
	{
		printf("ERROR: Can't open HXCSDFE.CFG !");
		return 0;
	}

	read_cfg_file(uicontext,(unsigned char*)&cfgfile_header);

	clear_slots(uicontext);

	save_cfg_file(uicontext,(unsigned char*)&cfgfile_header, -1);

	return 0;
}

char * supportedformat[]=
{
	"adf",
	"st",
	"hfe",
	"img",
	"dsk",
	0
};

int strcmpi(char* str1,char* str2)
{
	int i;

	i = 0;
	while( (tolower(str1[i]) == tolower(str2[i]))  && str1[i] && str2[i])
	{
		i++;
	}

	if( !str1[i] && !str2[i] )
	{
		return 0;
	}

	return 1;
}

int isSupported(char * ext)
{
	int i;

	i = 0;
	strlwr(ext);

	while(supportedformat[i])
	{
		if(!strcmp(supportedformat[i],ext))
			return 1;
		i++;
	}
	return 0;
}

int scan_tree(ui_context * uicontext)
{
	struct fs_dir_ent dir_entry;
	FL_DIR file_list_status;
	int pathlen,slot,slotnumber,j;
	disk_in_drive_v2_long DirectoryEntry;
	int drive,freeslotfound;

	fl_opendir(scanfolderpath, &file_list_status);
	while(!fl_readdir(&file_list_status, &dir_entry))
	{
		if(dir_entry.is_dir)
		{
			#ifdef DEBUG
			printf("Folder Entry : %s Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.cluster));
			#endif

			if(strcmp(dir_entry.filename,".") && strcmp(dir_entry.filename,".."))
			{
				pathlen = strlen(scanfolderpath);
				strcat(scanfolderpath,"/");
				strcat(scanfolderpath,dir_entry.filename);
				if( scan_tree( uicontext ) )
					return 1;
				scanfolderpath[pathlen] = 0;
			}
		}
		else
		{
			#ifdef DEBUG
			printf("File Entry : %s Size:%d Cluster 0x%.8X\n",dir_entry.filename,ENDIAN_32BIT(dir_entry.size),ENDIAN_32BIT(dir_entry.cluster));
			#endif

			if(strcmpi(dir_entry.filename,"autoboot.hfe"))
			{

				slot = get_slot_from_cluster(uicontext,ENDIAN_32BIT(dir_entry.cluster));

				if( slot < 0 )
				{
					// If not found...
					// Add it.
					// Search a free slot...
					drive = 0;
					freeslotfound = 0;
					slotnumber = 1;
					// Get the file name extension.
					getext(dir_entry.filename,(char*)&DirectoryEntry.type);
					if(isSupported((char*)&DirectoryEntry.type))
					{
						printf("Adding %s... ",dir_entry.filename);
						do
						{
							if( !(uicontext->slot_map[slotnumber>>3] & (0x80 >> (slotnumber&7))) )
							{
								// Free slot found...
								uicontext->slot_map[slotnumber>>3]   |= (0x80 >> (slotnumber&7));
								uicontext->change_map[slotnumber>>3] |= (0x80 >> (slotnumber&7));

								DirectoryEntry.attributes=0x00;
								if(dir_entry.is_readonly)
									DirectoryEntry.attributes |= FILE_ATTR_READ_ONLY;

								if(dir_entry.is_system)
									DirectoryEntry.attributes |= FILE_ATTR_SYSTEM;

								if(dir_entry.is_hidden)
									DirectoryEntry.attributes |= FILE_ATTR_HIDDEN;

								j = 0;
								while(j<MAX_LONG_NAME_LENGHT && dir_entry.filename[j])
								{
									DirectoryEntry.name[j] = dir_entry.filename[j];
									j++;
								}

								DirectoryEntry.name[j] = 0x00;

								DirectoryEntry.firstCluster = ENDIAN_32BIT(dir_entry.cluster) ;
								DirectoryEntry.size = ENDIAN_32BIT(dir_entry.size);

								memcpy( (void*)&disks_slots[ (slotnumber * uicontext->number_of_drive) + drive ],
										(void*)&DirectoryEntry,
										sizeof(disk_in_drive_v2)
										);
								freeslotfound = 1;

								printf("to slot %d\n",slotnumber);
							}

							slotnumber++;
						}while(slotnumber < uicontext->config_file_number_max_of_slot && !freeslotfound);

						if(!freeslotfound)
						{
							printf("\nError : Can't add %s ! No free space into this cfg file !\n",dir_entry.filename);
						}
					}
				}
			}
		}
	}
	return 0;
}

int auto_populate_slots()
{
	ui_context * uicontext;
	unsigned char cfgfile_header[512];
	uicontext = &g_ui_ctx;

	memset( uicontext,0,sizeof(ui_context));
	strcpy( uicontext->currentPath, "/" );

	init_fdc(0);

	attach_and_init_media();

	printf("Reading HXCSDFE.CFG ...\n");

	cfg_file_handle = fl_fopen("/HXCSDFE.CFG", "r");
	if (!cfg_file_handle)
	{
		printf("ERROR: Can't open HXCSDFE.CFG !");
		return 0;
	}

	read_cfg_file(uicontext,(unsigned char*)&cfgfile_header);

	scanfolderpath[0] = 0;
	scanfolderpath[1] = 0;
	scan_tree(uicontext);

	save_cfg_file(uicontext,(unsigned char*)&cfgfile_header, -1);

	return 0;
}