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

#include <devices/trackdisk.h>
#include <dos/dos.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/commodities.h>

#include <proto/commodities.h>
#include <proto/exec.h>

//#include <pragmas/commodities_pragmas.h>
//#include <pragmas/exec_pragmas.h>

#include <intuition/screens.h>
#include <intuition/preferences.h>

#include <exec/interrupts.h>

#include <stdio.h>
#include <string.h>

#include "keysfunc_defs.h"

#include "gui_utils.h"
#include "cfg_file.h"
#include "hxcfeda.h"


#include "amiga_hw.h"
#include "amiga_regs.h"

#include "fat_opts.h"
#include "fat_misc.h"
#include "fat_defs.h"
#include "fat_filelib.h"

#include "conf.h"

//#define DBGMODE 1
#define  INTB_PORTS	(3)

static unsigned long indexptr;
static unsigned short y_pos;
static unsigned long last_setlbabase;
static unsigned long cluster;
static unsigned char sector[512];
static unsigned char cfgfile_header[512];

static unsigned char currentPath[4*256] = {"\\"};

static unsigned char sdfecfg_file[2048];
static unsigned char filename[4097];
static char filter[17];


static unsigned char slotnumber;
static char selectorpos;
static short slotselectorpos;
static short slotselectorpage;
static unsigned char read_entry;

static disk_in_drive disks_slot_a[MAX_NUMBER_OF_SLOT];
static disk_in_drive disks_slot_b[MAX_NUMBER_OF_SLOT];
static DirectoryEntry DirectoryEntry_tab[40];

static struct fs_dir_list_status file_list_status;
static struct fs_dir_list_status file_list_status_tab[512];
static struct fat_dir_entry sfEntry;
static struct fs_dir_ent dir_entry;
extern struct fatfs _fs;

extern unsigned short SCREEN_YRESOL;
extern unsigned char  NUMBER_OF_FILE_ON_DISPLAY;

struct Interrupt *rbfint, *priorint;
unsigned long timercnt,config_file_number_max_of_slot;
unsigned char bkstr[40][80+8];
extern unsigned char keyup;

volatile unsigned short io_floppy_timeout;

void print_hex(unsigned char * buffer, int size)
{
	int c,i;
	int x,y;

	c=0;

	x=0;
	y=0;
	for(i=0;i<size;i++)
	{
		x=((c & 0xF)*24);
		hxc_printf(0,x,y,"%.2X ", buffer[i]);
		c++;
		if(!(c&0xF))
		{
			y=y+9;
		}
	}

	c=0;

	x=0;
	y=0;

	for(i=0;i<size;i++)
	{
		x=((c & 0xF)*8)+384+8;
		if(
			(buffer[i]>='a' && buffer[i]<='z') ||
			(buffer[i]>='A' && buffer[i]<='Z') ||
			(buffer[i]>='0' && buffer[i]<='9')
			)
		{
			hxc_printf(0,x,y,"%c", buffer[i]);
		}
		else
		{
			hxc_printf(0,x,y,".");
		}
		c++;
		if(!(c&0xF))
		{
			y=y+9;
		}
	}
}

void lockup()
{
	for(;;);
}

int setlbabase(unsigned long lba)
{
	int ret;
	unsigned char cmd_cnt;
	unsigned long lbatemp;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- setlbabase E --");
	#endif

	direct_access_cmd_sector * dacs;
	direct_access_status_sector * dass;

	dass=(direct_access_status_sector *)sector;
	dacs=(direct_access_cmd_sector  *)sector;

	memset(&sector,0,512);

	sprintf(dacs->DAHEADERSIGNATURE,"HxCFEDA");
	dacs->cmd_code=1;
	dacs->parameter_0=(lba>>0)&0xFF;
	dacs->parameter_1=(lba>>8)&0xFF;
	dacs->parameter_2=(lba>>16)&0xFF;
	dacs->parameter_3=(lba>>24)&0xFF;
	dacs->parameter_4=0xA5;

	ret=writesector( 0,(unsigned char *)&sector);
	if(!ret)
	{
		hxc_printf_box(0,"ERROR: Write CTRL ERROR !");
		lockup();
	}

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- setlbabase L --");
	#endif

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
			hxc_printf_box(0,"read sector %d error !",last_setlbabase);
			for(;;);
		}

		#ifdef DBGMODE
			hxc_printf(0,0,0,"       %.8X = %.8X ?" ,last_setlbabase,L_INDIAN(dass->lba_base));
		#endif

		if(last_setlbabase!=L_INDIAN(dass->lba_base))
		{
			hxc_printf_box(0,"LBA Change Test Failed ! Write Issue ?");
			for(;;);
		}

		last_setlbabase--;
	}while(last_setlbabase);

	return 0;
}

int media_init()
{
	unsigned char ret;
	unsigned char sector[512];
	int i;
	direct_access_status_sector * dass;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_init E --");
	#endif

	last_setlbabase=0xFFFFF000;
	ret=readsector(0,(unsigned char*)&sector,1);

	if(ret)
	{
		dass=(direct_access_status_sector *)sector;
		if(!strcmp(dass->DAHEADERSIGNATURE,"HxCFEDA"))
		{
			hxc_printf(0,0,SCREEN_YRESOL-30,"Firmware %s" ,dass->FIRMWAREVERSION);

			test_floppy_if();

			dass= (direct_access_status_sector *)sector;
			last_setlbabase=0;
			setlbabase(last_setlbabase);

			#ifdef DBGMODE
				hxc_printf(0,0,0,"-- media_init L --");
			#endif

			return 1;
		}

		hxc_printf_box(0,"Bad signature - HxC Floppy Emulator not found!");

		#ifdef DBGMODE
			hxc_printf(0,0,0,"-- media_init L --");
		#endif

		return 0;
	}

	hxc_printf_box(0,"ERROR: Floppy Access error!  [%d]",ret);

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_init L --");
	#endif

	return 0;
}

int media_read(unsigned long sector, unsigned char *buffer)
{
	int ret,retry;
	direct_access_status_sector * dass;

	dass= (direct_access_status_sector *)buffer;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_read E --");
	#endif


	hxc_printf(0,8*79,0,"%c",23);

	ret=0;

	do
	{
		if((sector-last_setlbabase)>=8)
		{
			setlbabase(sector);
		}

		if(!readsector(0,buffer,0))
		{
			hxc_printf_box(0,"ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
		}
		last_setlbabase = L_INDIAN(dass->lba_base);

	}while((sector-L_INDIAN(dass->lba_base))>=8);

	if(!readsector((sector-last_setlbabase)+1,buffer,0))
	{
		hxc_printf_box(0,"ERROR: Read ERROR ! fsector %d",(sector-last_setlbabase)+1);
		lockup();
	}

	hxc_printf(0,8*79,0," ");

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_read L --");
	#endif

	return 1;
}

int media_write(unsigned long sector, unsigned char *buffer)
{
	int ret,retry;
	direct_access_status_sector * dass;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_write E --");
	#endif

	hxc_printf(0,8*79,0,"%c",23);

	if((sector-last_setlbabase)>=8)
	{
		last_setlbabase=sector;
		setlbabase(sector);
	}

	if(!writesector((sector-last_setlbabase)+1,buffer))
	{
		hxc_printf_box(0,"ERROR: Write sector ERROR !");
		lockup();
	}

	hxc_printf(0,8*79,0," ");

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- media_write L --");
	#endif

	return 1;
}

void printslotstatus(unsigned char slotnumber,  disk_in_drive * disks_a,  disk_in_drive * disks_b)
{
	char tmp_str[17];

	hxc_printf(0,0,SLOT_Y_POS,"Slot %.2d:", slotnumber);

	//clear_line(SLOT_Y_POS+8,0);
	hxc_printf(0,0,SLOT_Y_POS+8,"Drive A:                 ");
	if( disks_a->DirEnt.size)
	{
		memcpy(tmp_str,disks_a->DirEnt.longName,16);
		tmp_str[16]=0;
		hxc_printf(0,0,SLOT_Y_POS+8,"Drive A: %s", tmp_str);
	}

	//clear_line(SLOT_Y_POS+16,0);
	hxc_printf(0,0,SLOT_Y_POS+16,"Drive B:                 ");
	if(disks_b->DirEnt.size)
	{
		memcpy(tmp_str,disks_b->DirEnt.longName,16);
		tmp_str[16]=0;
		hxc_printf(0,0,SLOT_Y_POS+16,"Drive B: %s", tmp_str);
	}
};

char read_cfg_file(unsigned char * sdfecfg_file)
{
	char ret;
	unsigned char number_of_slot;
	unsigned short i;
	int file_cfg_size;
	cfgfile * cfgfile_ptr;
	FL_FILE *file;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- read_cfg_file E --");
	#endif

	memset((void*)&disks_slot_a,0,sizeof(disk_in_drive)*MAX_NUMBER_OF_SLOT);
	memset((void*)&disks_slot_b,0,sizeof(disk_in_drive)*MAX_NUMBER_OF_SLOT);

	ret=0;
	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		fl_fseek( file, 0, SEEK_END );
		file_cfg_size = fl_ftell( file );
		config_file_number_max_of_slot = ( ( file_cfg_size - 0x400 ) / (64 * 2) ) - 1;
		fl_fseek( file, 0, SEEK_SET );
		if( config_file_number_max_of_slot > MAX_NUMBER_OF_SLOT )
			config_file_number_max_of_slot = MAX_NUMBER_OF_SLOT;

		cfgfile_ptr=(cfgfile * )cfgfile_header;

		fl_fread(cfgfile_header, 1, 512 , file);
		number_of_slot = cfgfile_ptr->number_of_slot;
		
		if( number_of_slot > MAX_NUMBER_OF_SLOT )
			number_of_slot = MAX_NUMBER_OF_SLOT - 1;
		
		if( number_of_slot > config_file_number_max_of_slot )
			number_of_slot = config_file_number_max_of_slot - 1;

		fl_fseek(file , 1024 , SEEK_SET);

		fl_fread(sdfecfg_file, 1, 512 , file);
		i=1;
		do
		{
			if(!(i&3))
			{
				fl_fread(sdfecfg_file, 1, 512 , file);
			}

			memcpy(&disks_slot_a[i],&sdfecfg_file[(i&3)*128],sizeof(disk_in_drive));
			memcpy(&disks_slot_b[i],&sdfecfg_file[((i&3)*128)+64],sizeof(disk_in_drive));

			i++;
		}while(i<number_of_slot);

		fl_fclose(file);
	}
	else
	{
		ret=1;
	}

	if(ret)
	{
		hxc_printf_box(0,"ERROR: Access HXCSDFE.CFG file failed! [%d]",ret);
	}

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- read_cfg_file L --");
	#endif

	return ret;
}

char save_cfg_file(unsigned char * sdfecfg_file)
{
	unsigned char number_of_slot,slot_index;
	unsigned char i,sect_nb,ret;
	cfgfile * cfgfile_ptr;
	unsigned short  floppyselectorindex;
	FL_FILE *file;

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- save_cfg_file E --");
	#endif

	ret=0;
	file = fl_fopen("/HXCSDFE.CFG", "r");
	if (file)
	{
		number_of_slot=1;
		slot_index=1;
		i=1;

		floppyselectorindex=128;                      // Fisrt slot offset
		memset( sdfecfg_file,0,512);                  // Clear the sector
		sect_nb=2;                                    // Slots Sector offset

		do
		{
			if( disks_slot_a[i].DirEnt.size )            // Valid slot found
			{
				// Copy it to the actual file sector
				memcpy(&sdfecfg_file[floppyselectorindex],&disks_slot_a[i],sizeof(disk_in_drive));
				memcpy(&sdfecfg_file[floppyselectorindex+64],&disks_slot_b[i],sizeof(disk_in_drive));

				//Next slot...
				number_of_slot++;
				floppyselectorindex=(floppyselectorindex+128)&0x1FF;

				if(!(number_of_slot&0x3))                // Need to change to the next sector
				{
					// Save the sector
					if (fl_fswrite((unsigned char*)sdfecfg_file, 1,sect_nb, file) != 1)
					{
						hxc_printf_box(0,"ERROR: Write file failed!");
						ret=1;
					}
					// Next sector
					sect_nb++;
					memset( sdfecfg_file,0,512);                  // Clear the next sector
				}
			}

			i++;
		}while(i<config_file_number_max_of_slot);

		if(number_of_slot&0x3)
		{
			if (fl_fswrite((unsigned char*)sdfecfg_file, 1,sect_nb, file) != 1)
			{
				hxc_printf_box(0,"ERROR: Write file failed!");
				ret=1;
			}
		}

		if(slot_index>=number_of_slot)
		{
			slot_index=number_of_slot-1;
		}

		fl_fseek(file , 0 , SEEK_SET);

		// Update the file header
		cfgfile_ptr=(cfgfile * )cfgfile_header;
		cfgfile_ptr->number_of_slot=number_of_slot;
		cfgfile_ptr->slot_index=slot_index;

		if (fl_fswrite((unsigned char*)cfgfile_header, 1,0, file) != 1)
		{
			hxc_printf_box(0,"ERROR: Write file failed!");
			ret=1;
		}

	}
	else
	{
		hxc_printf_box(0,"ERROR: Create file failed!");
		ret=1;
	}

	// Close file
	fl_fclose(file);

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- save_cfg_file L --");
	#endif

	return ret;
}

void clear_list(unsigned char add)
{
	unsigned char y_pos,i;

	y_pos=FILELIST_Y_POS;
	for(i=0;i<NUMBER_OF_FILE_ON_DISPLAY+add;i++)
	{
		clear_line(y_pos,0);
		y_pos=y_pos+8;
	}
}

void next_slot()
{
	slotnumber++;
	if(slotnumber> ( config_file_number_max_of_slot - 1) )  slotnumber=1;
	printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;
}

void displayFolder()
{
	int i;
	hxc_printf(0,SCREEN_XRESOL/2,CURDIR_Y_POS,"Current directory:");

	for(i=SCREEN_XRESOL/2;i<SCREEN_XRESOL;i=i+8) hxc_printf(0,i,CURDIR_Y_POS+8," ");

	if(strlen(currentPath)<32)
		hxc_printf(0,SCREEN_XRESOL/2,CURDIR_Y_POS+8,"%s",currentPath);
	else
        hxc_printf(0,SCREEN_XRESOL/2,CURDIR_Y_POS+8,"...%s    ",&currentPath[strlen(currentPath)-32]);
}

void enter_sub_dir(disk_in_drive *disk_ptr)
{
	unsigned long first_cluster;
	int currentPathLength;
	unsigned char folder[128+1];
	unsigned char c;
	int i;
	int old_index;

	old_index=strlen( currentPath );

	if ( (disk_ptr->DirEnt.longName[0] == (unsigned char)'.') && (disk_ptr->DirEnt.longName[1] == (unsigned char)'.') )
	{
		currentPathLength = strlen( currentPath ) - 1;
		do
		{
			currentPath[ currentPathLength ] = 0;
			currentPathLength--;
		}
		while ( currentPath[ currentPathLength ] != (unsigned char)'/' );
	}
	else
	{
		if((disk_ptr->DirEnt.longName[0] == (unsigned char)'.'))
		{
		}
		else
		{
			for (i=0; i < 128; i++ )
			{
				c = disk_ptr->DirEnt.longName[i];
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

			currentPathLength = strlen( currentPath );

			if( currentPath[ currentPathLength-1] != '/')
			strcat( currentPath, "/" );

			strcat( currentPath, folder );
		}

	}

	displayFolder();

	selectorpos=0;

	if(!fl_list_opendir(currentPath, &file_list_status))
	{
		currentPath[old_index]=0;
		fl_list_opendir(currentPath, &file_list_status);
		displayFolder();
	}
	for(i=0;i<512;i++)
	{
		memcpy(&file_list_status_tab[i],&file_list_status ,sizeof(struct fs_dir_list_status));
	}
 	clear_list(0);
	read_entry=1;
}

void show_all_slots(int drive)
{
	char tmp_str[81];
	disk_in_drive * drive_slots_ptr;
	unsigned short i;

	hxc_printf(1,0,FILELIST_Y_POS,"--- Drive (%c) selection ---",'A'+drive);

	switch(drive)
	{
		case 0:
			drive_slots_ptr = disks_slot_a;
		break;
		case 1:
			drive_slots_ptr = disks_slot_b;
		break;
		default:
			drive_slots_ptr = disks_slot_a;
		break;
	}

	for ( i = 1; i < NUMBER_OF_FILE_ON_DISPLAY; i++ )
	{
		if(i + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)) < config_file_number_max_of_slot)
		{
			tmp_str[0]=0; 
			if( drive_slots_ptr[i + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))].DirEnt.size)
			{
				memcpy(tmp_str,&drive_slots_ptr[i + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))].DirEnt.longName,41);	
			}
			tmp_str[80]=0;
			hxc_printf(0,0,FILELIST_Y_POS + (i*8),"%.3d:%s", i + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)), tmp_str);
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
		exttodest[3] = 0;
	}

	return 0;
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

void restorestr()
{
	int i;

	for(i=0;i<NUMBER_OF_FILE_ON_DISPLAY;i++)
	{
		hxc_printf(0,0,FILELIST_Y_POS+(i*8),bkstr[i+1]);
	}

	invert_line(0,FILELIST_Y_POS+(selectorpos*8));
}

int main(int argc, char* argv[])
{
	unsigned short i,page_number;
	unsigned char key, entrytype,bootdev,j;
	unsigned char last_file,filtermode,displayentry,c;
	disk_in_drive * disk_ptr;
	cfgfile * cfgfile_ptr;
	unsigned char colormode;
	int slots_list_drive;

	FILE *f;

	init_display();

	rbfint = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR);
	rbfint->is_Node.ln_Type = NT_INTERRUPT;      /* Init interrupt node. */
	rbfint->is_Node.ln_Name = "HxCFESelectorTimerInt";
	rbfint->is_Data = 0;//(APTR)rbfdata;
	rbfint->is_Code = ithandler;

	AddIntServer(5,rbfint);
	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- Init display Done --");
	#endif

	io_floppy_timeout = 0;
	selectorpos = 0;
	slotselectorpos = 0;
	slotselectorpage = 0;
	config_file_number_max_of_slot = 0;

	bootdev = 0;
	while( bootdev<4 && !test_drive(bootdev) )
	{
		bootdev++;
	}

	if(bootdev >= 4)
	{
		bootdev = 0;
	}

	init_amiga_fdc(bootdev);

	#ifdef DBGMODE
		hxc_printf(0,0,0,"-- init_amiga_fdc Done --");
	#endif

	if(media_init())
	{
		#ifdef DBGMODE
			hxc_printf(0,0,0,"-- media_init done --");
		#endif

		// Initialise File IO Library
		fl_init();

		#ifdef DBGMODE
			hxc_printf(0,0,0,"-- fl_init done --");
		#endif

		/* Attach media access functions to library*/
		if (fl_attach_media(media_read, media_write) != FAT_INIT_OK)
		{
			hxc_printf_box(0,"ERROR: Media attach failed !");
			for(;;);
		}

		#ifdef DBGMODE
			hxc_printf(0,0,0,"-- fl_attach_media done --");
		#endif

		hxc_printf_box(0,"Reading HXCSDFE.CFG ...");

		read_cfg_file(sdfecfg_file);

		#ifdef DBGMODE
			hxc_printf(0,0,0,"-- read_cfg_file done --");
		#endif

		if(cfgfile_header[256+128]!=0xFF)
			set_color_scheme(cfgfile_header[256+128]);

		strcpy( currentPath, "/" );
		displayFolder();

		slotnumber=1;
		printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;

		colormode=0;
		read_entry=0;
		selectorpos=0;
		slots_list_drive = 0;
		cluster=fatfs_get_root_cluster(&_fs);
		page_number=0;
		last_file=0;
		filtermode=0;
		fl_list_opendir(currentPath, &file_list_status);
		for(i=0;i<512;i++)
		{
			memcpy(&file_list_status_tab[i],&file_list_status ,sizeof(struct fs_dir_list_status));
		}
		clear_list(0);

		for(;;)
		{
			y_pos=FILELIST_Y_POS;
			for(;;)
			{
				i=0;
				do
				{
					memset(&DirectoryEntry_tab[i],0,sizeof(DirectoryEntry));
					i++;
				}while((i<NUMBER_OF_FILE_ON_DISPLAY));

				i=0;
				memset(bkstr,0,sizeof(bkstr));
				y_pos=FILELIST_Y_POS;
				last_file=0x00;
				do
				{
					displayentry=0xFF;
					if(fl_list_readdir(&file_list_status, &dir_entry))
					{
						if(filtermode)
						{
							strlwr(dir_entry.filename);

							if(!strstr(dir_entry.filename,filter))
							{
								displayentry=0x00;
							}
						}

						if(displayentry)
						{
							if(dir_entry.is_dir)
							{
								entrytype=10;
								DirectoryEntry_tab[i].attributes=0x10;
							}
							else
							{
								entrytype=12;
								DirectoryEntry_tab[i].attributes=0x00;
							}

							snprintf(bkstr[y_pos/8],80," %c%s",entrytype,dir_entry.filename);
							hxc_printf(0,0,y_pos," %c%s",entrytype,dir_entry.filename);

							y_pos=y_pos+8;
							dir_entry.filename[127]=0;
							sprintf(DirectoryEntry_tab[i].longName,"%s",dir_entry.filename);

							// Get the short name - with the file name extension !
							DirectoryEntry_tab[i].name[8+3] = 0;
							memset(DirectoryEntry_tab[i].name,' ',8+3);
							getext(dir_entry.filename,&DirectoryEntry_tab[i].name[8]);

							j = 0;
							while(j<8 && dir_entry.filename[j] != 0 && dir_entry.filename[j] != '.')
							{
								DirectoryEntry_tab[i].name[j] = dir_entry.filename[j];
								j++;
							}

							DirectoryEntry_tab[i].firstCluster = ENDIAN_32BIT(dir_entry.cluster) ;
							DirectoryEntry_tab[i].size =  ENDIAN_32BIT(dir_entry.size);
							i++;
						}

					}
					else
					{
						last_file=0xFF;
						i=NUMBER_OF_FILE_ON_DISPLAY;
					}

				}while((i<NUMBER_OF_FILE_ON_DISPLAY) && Keyboard()!=0x45);

				filtermode=0;

				memcpy(&file_list_status_tab[(page_number+1)&0x1FF],&file_list_status ,sizeof(struct fs_dir_list_status));

				hxc_printf(0,0,FILELIST_Y_POS+(selectorpos*8),">");
				invert_line(0,FILELIST_Y_POS+(selectorpos*8));

				read_entry=0;

				do
				{

					key=wait_function_key();
					if(1)
					{
						switch(key)
						{
						case FCT_UP_KEY: // UP
							invert_line(0,FILELIST_Y_POS+(selectorpos*8));
							hxc_printf(0,0,FILELIST_Y_POS+(selectorpos*8)," ");

							selectorpos--;
							if(selectorpos<0)
							{
								selectorpos=NUMBER_OF_FILE_ON_DISPLAY-1;
								if(page_number) page_number--;
								clear_list(0);
								read_entry=1;
								memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							}
							else
							{

								hxc_printf(0,0,FILELIST_Y_POS+(selectorpos*8),">");
								invert_line(0,FILELIST_Y_POS+(selectorpos*8));
							}
							break;

						case FCT_DOWN_KEY: // Down
							invert_line(0,FILELIST_Y_POS+(selectorpos*8));
							hxc_printf(0,0,FILELIST_Y_POS+(selectorpos*8)," ");

							selectorpos++;
							if(selectorpos>=NUMBER_OF_FILE_ON_DISPLAY)
							{
								selectorpos=0;
								clear_list(0);
								read_entry=1;
								if(!last_file)page_number++;
								memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							}
							else
							{
								hxc_printf(0,0,FILELIST_Y_POS+(selectorpos*8),">");
								invert_line(0,FILELIST_Y_POS+(selectorpos*8));
							}

							break;

						case FCT_RIGHT_KEY: // Right
							clear_list(0);
							read_entry=1;
							if(!last_file)page_number++;
							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));

							break;

						case FCT_LEFT_KEY:
							if(page_number) page_number--;
							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;
							break;

						case FCT_NEXTSLOT:
							next_slot();
							wait_released_key();
							break;

						case FCT_SAVE:
							hxc_printf_box(0,"Saving selection...");
							save_cfg_file(sdfecfg_file);
							restore_box();
							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;
							break;

						case FCT_SELECT_FILE_DRIVEB:
							slots_list_drive = 1;
						case FCT_SELECT_FILE_DRIVEA:
							disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

							if(disk_ptr->DirEnt.attributes&0x10)
							{
								enter_sub_dir(disk_ptr);
							}
							else
							{
								clear_list(0);
								show_all_slots(slots_list_drive);
								invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
								do
								{
									key=wait_function_key();

									switch(key)
									{
										case FCT_UP_KEY: // UP
											//invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
											if(slotselectorpos)
											{
												slotselectorpos--;
												invert_line(0,FILELIST_Y_POS+((slotselectorpos+1)*8));
												invert_line(0,FILELIST_Y_POS+((slotselectorpos)*8));
											}
											else
											{
												if(slotselectorpage)
												{
													slotselectorpos = (NUMBER_OF_FILE_ON_DISPLAY-1);
													slotselectorpage--;
												}

												clear_list(0);
												show_all_slots(slots_list_drive);
												invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
											}
										break;
										case FCT_DOWN_KEY: // Down
											
											if(slotselectorpos + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1)) < config_file_number_max_of_slot )
											{
												slotselectorpos++;
												if(slotselectorpos>(NUMBER_OF_FILE_ON_DISPLAY-1))
												{
													slotselectorpos = 1;
													slotselectorpage++;
													
													clear_list(0);
													show_all_slots(slots_list_drive);
													invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
												}
												else
												{
													invert_line(0,FILELIST_Y_POS+((slotselectorpos-1)*8));
													invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
												}
											}
										break;
										
										case FCT_RIGHT_KEY: // Right
											if(slotselectorpos + ((slotselectorpage+1) * (NUMBER_OF_FILE_ON_DISPLAY-1)) < config_file_number_max_of_slot )
											{
												slotselectorpage++;
											}
											clear_list(0);
											show_all_slots(slots_list_drive);
											invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
										break;

										case FCT_LEFT_KEY:
											if(slotselectorpage)
												slotselectorpage--;
											clear_list(0);
											show_all_slots(slots_list_drive);
											invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
										break;
										
										case FCT_CLEARSLOT:
											if(!slots_list_drive)
												memset((void*)&disks_slot_a[slotselectorpos + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))],0,sizeof(disk_in_drive));
											else
												memset((void*)&disks_slot_b[slotselectorpos + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))],0,sizeof(disk_in_drive));

											clear_list(0);
											show_all_slots(slots_list_drive);
											invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));
										break;
										
										case FCT_SELECT_FILE_DRIVEA:
											if(!slotselectorpos)
											{
												if(slots_list_drive)
													slots_list_drive = 0;
												else
													slots_list_drive = 1;
													
												key = 0;
												
												clear_list(0);
												show_all_slots(slots_list_drive);
												invert_line(0,FILELIST_Y_POS+(slotselectorpos*8));												
											}
										
										break;
										
									}
								}while(key != FCT_SELECT_FILE_DRIVEA);

								clear_list(0);
								show_all_slots(slots_list_drive);
								clear_list(0);
								restorestr();
								if(!slots_list_drive)
									memcpy((void*)&disks_slot_a[slotselectorpos + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
								else
									memcpy((void*)&disks_slot_b[slotselectorpos + (slotselectorpage * (NUMBER_OF_FILE_ON_DISPLAY-1))],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));


							}
							break;

						case FCT_SELECT_FILE_DRIVEA_AND_NEXTSLOT:
							disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

							if(disk_ptr->DirEnt.attributes&0x10)
							{
								enter_sub_dir(disk_ptr);
							}
							else
							{
								memcpy((void*)&disks_slot_a[slotnumber],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
								next_slot();
							}
							wait_released_key();
							break;

						case FCT_SELECTSAVEREBOOT:
							disk_ptr=(disk_in_drive * )&DirectoryEntry_tab[selectorpos];

							if(disk_ptr->DirEnt.attributes&0x10)
							{
								enter_sub_dir(disk_ptr);
							}
							else
							{
								memcpy((void*)&disks_slot_a[1],(void*)&DirectoryEntry_tab[selectorpos],sizeof(disk_in_drive));
								hxc_printf_box(0,"Saving selection and restart...");
								save_cfg_file(sdfecfg_file);
								restore_box();
								hxc_printf_box(0,">>>>>Rebooting...<<<<<");
								sleep(1);
								jumptotrack(0);
								reboot();

							}
							break;

						case FCT_HELP:
							clear_list(5);

							i=0;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Function Keys (1/2):");

							i=2;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Up/Down/Right/Left: Browse the SD/USB files");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Right Shift       : Go back to the top of the folder");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Left A / ENTER    : Insert the selected file in the current slot to A:");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "                    Enter a subfolder");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Right A           : Insert the selected file in the current slot to B:");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Left ALT          : Insert the selected file in the current slot to A: and ");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "                    select the next slot");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F7                : Insert the selected file in the slot to 1 and restart the");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "                    computer with this disk.");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Right ALT         : Select the the next slot");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "BACKSPACE         : Clear the current slot");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "DEL               : Clear the current slot and Select the the next slot");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "TAB               : Show all slots selections");

							i=i+2;

							hxc_printf(1,0,HELP_Y_POS+(i*8), "---Press Space to continue---");

							do
							{

							}while(wait_function_key()!=FCT_OK);

							clear_list(5);

							i=0;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Function Keys (2/2):");

							i=2;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F1                : Search files in the current folder");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "                    Type the word to search then enter");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "                    Excape to abord the search");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F2                : Change color");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F3                : Settings menu");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F8                : Reboot");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F9                : Save");
							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "F10               : Save and Reboot");

							i=i+2;

							hxc_printf(1,0,HELP_Y_POS+(i*8), "---Press Space to exit---");
							i=i+2;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "HxC Floppy Emulator file selector for Amiga");
							i++;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "(c) 2006-2015 HxC2001 / Jean-Francois DEL NERO");
							i++;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "Check for updates on :");
							i++;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "http://hxc2001.free.fr/floppy_drive_emulator/");
							i++;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "Email : hxc2001@free.fr");
							i++;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "V%s - %s",VERSIONCODE,DATECODE);


							do
							{

							}while(wait_function_key()!=FCT_OK);

       						clear_list(5);
							init_buffer();
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;
							displayFolder();

							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;

							break;

						case FCT_EMUCFG:
							clear_list(5);
							cfgfile_ptr=(cfgfile * )cfgfile_header;

							i=0;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "HxC Floppy Emulator settings:");

							i=2;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Track step sound :");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");

							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "User interface sound:");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d   ",cfgfile_ptr->buzzer_duty_cycle);

							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "LCD Backlight standby:");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s",cfgfile_ptr->back_light_tmr);

							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "SD/USB Standby:");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s",cfgfile_ptr->standby_tmr);

							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "DF1 drive :");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");

							i++;
							hxc_printf(0,0,HELP_Y_POS+(i*8), "Load AUTOBOOT.HFE at power up :");
							hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->startup_mode&0x04?"on":"off");

							i=i+2;
							hxc_printf(1,0,HELP_Y_POS+(i*8), "---Press Space to exit---");

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
										if(i<7) i++;
										invert_line(0,HELP_Y_POS+(i*8));
									break;
									case FCT_LEFT_KEY:
										invert_line(0,HELP_Y_POS+(i*8));
										switch(i)
										{
											case 2:
												cfgfile_ptr->step_sound=~cfgfile_ptr->step_sound;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");
											break;
											case 3:
												if(cfgfile_ptr->buzzer_duty_cycle) cfgfile_ptr->buzzer_duty_cycle--;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d  ",cfgfile_ptr->buzzer_duty_cycle);
												if(!cfgfile_ptr->buzzer_duty_cycle) cfgfile_ptr->ihm_sound=0x00;
												break;
											case 4:
												if(cfgfile_ptr->back_light_tmr) cfgfile_ptr->back_light_tmr--;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->back_light_tmr);
											break;

											case 5:
												if(cfgfile_ptr->standby_tmr) cfgfile_ptr->standby_tmr--;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->standby_tmr);
											break;

											case 6:
												cfgfile_ptr->enable_drive_b=~cfgfile_ptr->enable_drive_b;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");
											break;

											case 7:
												cfgfile_ptr->startup_mode = cfgfile_ptr->startup_mode  ^ 0x04;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode&0x4)?"on":"off");
											break;
										}
										invert_line(0,HELP_Y_POS+(i*8));

									break;
									case FCT_RIGHT_KEY:
										invert_line(0,HELP_Y_POS+(i*8));
										switch(i)
										{
											case 2:
												cfgfile_ptr->step_sound=~cfgfile_ptr->step_sound;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->step_sound?"on":"off");
												break;
											case 3:
												if(cfgfile_ptr->buzzer_duty_cycle<0x80) cfgfile_ptr->buzzer_duty_cycle++;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d  ",cfgfile_ptr->buzzer_duty_cycle);
												cfgfile_ptr->ihm_sound=0xFF;
											break;
											case 4:
												if(cfgfile_ptr->back_light_tmr<0xFF) cfgfile_ptr->back_light_tmr++;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->back_light_tmr);

											break;
											case 5:
												if(cfgfile_ptr->standby_tmr<0xFF) cfgfile_ptr->standby_tmr++;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%d s ",cfgfile_ptr->standby_tmr);
											break;

											case 6:
												cfgfile_ptr->enable_drive_b=~cfgfile_ptr->enable_drive_b;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",cfgfile_ptr->enable_drive_b?"off":"on");
											break;

											case 7:
												cfgfile_ptr->startup_mode = cfgfile_ptr->startup_mode  ^ 0x04;
												hxc_printf(0,SCREEN_XRESOL/2,HELP_Y_POS+(i*8), "%s ",(cfgfile_ptr->startup_mode&0x4)?"on":"off");
											break;
										}
										invert_line(0,HELP_Y_POS+(i*8));
									break;

								}
							}while(c!=FCT_OK && !(Joystick()&0x10));

							clear_list(5);
							init_buffer();
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;
							displayFolder();

							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;

						break;

						case FCT_SHOWSLOTS:
							clear_list(5);
							show_all_slots(0);
							clear_list(5);

							init_buffer();
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;
							displayFolder();

							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;
							break;

						case FCT_CLEARSLOT:
							memset((void*)&disks_slot_a[slotnumber],0,sizeof(disk_in_drive));
							memset((void*)&disks_slot_b[slotnumber],0,sizeof(disk_in_drive));
							printslotstatus(slotnumber, (disk_in_drive *) &disks_slot_a[slotnumber], (disk_in_drive *) &disks_slot_b[slotnumber]) ;
							break;

						case FCT_CLEARSLOT_AND_NEXTSLOT:
							memset((void*)&disks_slot_a[slotnumber],0,sizeof(disk_in_drive));
							memset((void*)&disks_slot_b[slotnumber],0,sizeof(disk_in_drive));
							next_slot();
							wait_released_key();
							break;

						case FCT_SAVEREBOOT:
							hxc_printf_box(0,"Saving selection and restart...");
							save_cfg_file(sdfecfg_file);
							restore_box();
							hxc_printf_box(0,">>>>>Rebooting...<<<<<");
							sleep(1);
							jumptotrack(0);
							reboot();
							break;

						case FCT_REBOOT:
							hxc_printf_box(0,">>>>>Rebooting...<<<<<");
							sleep(1);
							jumptotrack(0);
							reboot();
							break;

						case FCT_CHGCOLOR:
							colormode++;
							set_color_scheme(colormode);
							cfgfile_header[256+128]=colormode;
							wait_released_key();
						break;
						case FCT_TOP:
							page_number=0;
							memcpy(&file_list_status ,&file_list_status_tab[page_number&0x1FF],sizeof(struct fs_dir_list_status));
							clear_list(0);
							read_entry=1;
							break;
						case FCT_SEARCH:
							filtermode=0xFF;
							hxc_printf(0,SCREEN_XRESOL/2,CURDIR_Y_POS+16,"Search:                     ");
							flush_char();
							i=0;
							do
							{
								filter[i]=0;
								c=get_char();
								if(c!='\n')
								{
									filter[i]=c;
									hxc_printf(0,SCREEN_XRESOL/2+(8*8)+(8*i),CURDIR_Y_POS+16,"%c",c);
								}
								i++;
							}while(c!='\n' && i<16);
							filter[i]=0;


							//get_str(&filter);
							strlwr(filter);
							hxc_printf(0,SCREEN_XRESOL/2+(8*8),CURDIR_Y_POS+16,"[%s]",filter);
							selectorpos=0;
							page_number=0;
							memcpy(&file_list_status ,&file_list_status_tab[0],sizeof(struct fs_dir_list_status));

							clear_list(0);
							read_entry=1;
						break;

						default:
							//printf("err %d!\n",key);
							break;
						}
					}
				}while(!read_entry);
			}
		}
	}

for(;;);

return 0;
}
