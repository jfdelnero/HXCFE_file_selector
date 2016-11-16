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

typedef struct cfgfile_
{
    char signature[16];                    //"HXCFECFGV1.0" or "HXCFECFGV2.0"
    unsigned char step_sound;              //0x00 -> off 0xFF->on
    unsigned char ihm_sound;               //0x00 -> off 0xFF->on
    unsigned char back_light_tmr;          //0x00 always off, 0xFF always on, other -> on x second
    unsigned char standby_tmr;             //0xFF disable, other -> on x second
    unsigned char disable_drive_select;
    unsigned char buzzer_duty_cycle;
    unsigned char number_of_slot;
    unsigned char slot_index;
    unsigned short update_cnt;
    unsigned char load_last_floppy;
    unsigned char buzzer_step_duration;
    unsigned char lcd_scroll_speed;
    unsigned char startup_mode;            // 0x01 -> In normal mode auto load STARTUPA.HFE at power up
                                           // 0x02 -> In normal mode auto load STARTUPB.HFE at power up
                                           // 0x04 -> In slot mode use slot 0 at power up (ignore index)
                                           // 0x08 -> Pre increment index when inserting the sdcard (no button/lcd mode)
    unsigned char enable_drive_b;
    unsigned char index_mode;

    unsigned char cfg_from_cfg_drive0;
    unsigned char interfacemode_drive0;
    unsigned char pin02_cfg_drive0;
    unsigned char pin34_cfg_drive0;

    unsigned char cfg_from_cfg_drive1;
    unsigned char interfacemode_drive1;
    unsigned char pin02_cfg_drive1;
    unsigned char pin34_cfg_drive1;

    unsigned char drive_b_as_motor_on;

    unsigned char padding_1[7];
    unsigned char padding_2[16];

    // V2.X Extension
    unsigned long slots_map_position;       // Slots map position into the file (sector number offset)
    unsigned long max_slot_number;          // Note : Map size in sector : ( ( max_slot_number / 8 ) / 512 ) + 1
    unsigned long slots_position;           // Slots position into the file (sector number)
    unsigned long number_of_drive_per_slot; // 2 by default.
    unsigned long cur_slot_number;          // Current position

}__attribute__((__packed__)) cfgfile;


struct ShortDirectoryEntry {
    unsigned char name[12];
    unsigned char attributes;
    unsigned long firstCluster;
    unsigned long size;
    unsigned char longName[17];	// boolean
}__attribute__((__packed__));

extern struct DirectoryEntry directoryEntry;


typedef struct disk_in_drive_
{
	struct ShortDirectoryEntry DirEnt;
	unsigned char numberoftrack;
	unsigned char numberofside;
	unsigned short rpm;
	unsigned short bitrate;
	unsigned short tracklistoffset;
}__attribute__((__packed__)) disk_in_drive;

#define MAX_SHORT_NAME_LENGHT ( 64 - ( 3 + 1 + 4 + 4 ) )

typedef struct disk_in_drive_v2_
{ 
	unsigned char type[3];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char name[MAX_SHORT_NAME_LENGHT];
}__attribute__((__packed__)) disk_in_drive_v2;

#define MAX_LONG_NAME_LENGHT ( 256 - ( 3 + 1 + 4 + 4 ) )

typedef struct disk_in_drive_v2_long_
{ 
	unsigned char type[3];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char name[MAX_LONG_NAME_LENGHT]; //Max entry name : 256 - (4+4+1+3)
}__attribute__((__packed__)) disk_in_drive_v2_long;
