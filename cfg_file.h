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

#define START_MODE_LOAD_STARTUPA 0x01
#define START_MODE_LOAD_STARTUPB 0x02
#define START_MODE_SLOT_0 0x04
#define START_MODE_PREINC 0x08
#define START_MODE_DSKEJECTED 0x10

#ifdef WIN32
#pragma pack(1)
#endif

typedef struct cfgfile_
{
    char     signature[16];                 //"HXCFECFGV1.0" or "HXCFECFGV2.0"    --- 0x00
    uint8_t  step_sound;                    //0x00 -> off 0xFF->on                --- 0x10
    uint8_t  ihm_sound;                     //0x00 -> off 0xFF->on
    uint8_t  back_light_tmr;                //0x00 always off, 0xFF always on, other -> on x second
    uint8_t  standby_tmr;                   //0xFF disable, other -> on x second
    uint8_t  disable_drive_select;
    uint8_t  buzzer_duty_cycle;
    uint8_t  number_of_slot;
    uint8_t  slot_index;
    uint16_t update_cnt;
    uint8_t  load_last_floppy;
    uint8_t  buzzer_step_duration;
    uint8_t  lcd_scroll_speed;
    uint8_t  startup_mode;                 // 0x01 -> In normal mode auto load STARTUPA.HFE at power up
                                           // 0x02 -> In normal mode auto load STARTUPB.HFE at power up
                                           // 0x04 -> In slot mode use slot 0 at power up (ignore index)
                                           // 0x08 -> Pre increment index when inserting the sdcard (no button/lcd mode)
                                           // 0x10 -> Disk ejected at powerup
    uint8_t  enable_drive_b;
    uint8_t  index_mode;

    uint8_t  cfg_from_cfg_drive0;          // --- 0x20
    uint8_t  interfacemode_drive0;
    uint8_t  pin02_cfg_drive0;
    uint8_t  pin34_cfg_drive0;

    uint8_t  cfg_from_cfg_drive1;
    uint8_t  interfacemode_drive1;
    uint8_t  pin02_cfg_drive1;
    uint8_t  pin34_cfg_drive1;

    uint8_t  drive_b_as_motor_on;

    uint8_t  padding_1[7];
    uint8_t  padding_2[16];                // --- 0x30

    // V2.X Extension --- 0x40
    uint32_t slots_map_position;           // Slots map position into the file (sector number offset)
    uint32_t max_slot_number;              // Note : Map size in sector : ( ( max_slot_number / 8 ) / 512 ) + 1
    uint32_t slots_position;               // Slots position into the file (sector number)
    uint32_t number_of_drive_per_slot;     // 2 by default.
    uint32_t cur_slot_number;              // Current position --- 0x50
    uint32_t ihm_mode;                     // user interface mode

    uint8_t  padding_3[8];                 // --- 0x60

    uint8_t  padding_4[160];               // --- 0x100

    uint8_t  padding_5[128];               // --- 0x180   File selector Scratch pad area...

    uint8_t  background_color;

#ifndef WIN32
}__attribute__((__packed__)) cfgfile;
#else
}cfgfile;
#endif

struct ShortDirectoryEntry {
    char     name[12];
    uint8_t  attributes;
    uint32_t firstCluster;
    uint32_t size;
    char     longName[17];	// boolean
#ifndef WIN32
}__attribute__((__packed__));
#else
};
#endif

extern struct DirectoryEntry directoryEntry;

typedef struct disk_in_drive_
{
    struct   ShortDirectoryEntry DirEnt;
    uint8_t  numberoftrack;
    uint8_t  numberofside;
    uint16_t rpm;
    uint16_t bitrate;
    uint16_t tracklistoffset;
#ifndef WIN32
}__attribute__((__packed__)) disk_in_drive;
#else
}disk_in_drive;
#endif

#define MAX_SHORT_NAME_LENGHT ( 64 - ( 3 + 1 + 4 + 4 ) )

typedef struct disk_in_drive_v2_
{
    char     type[3];
    uint8_t  attributes;
    uint32_t firstCluster;
    uint32_t size;
    char     name[MAX_SHORT_NAME_LENGHT];
#ifndef WIN32
}__attribute__((__packed__)) disk_in_drive_v2;
#else
}disk_in_drive_v2;
#endif

#define MAX_LONG_NAME_LENGHT ( 256 - ( 3 + 1 + 4 + 4 ) )

typedef struct disk_in_drive_v2_long_
{
    char     type[3];
    uint8_t  attributes;
    uint32_t firstCluster;
    uint32_t size;
    char     name[MAX_LONG_NAME_LENGHT]; //Max entry name : 256 - (4+4+1+3)
#ifndef WIN32
}__attribute__((__packed__)) disk_in_drive_v2_long;
#else
}disk_in_drive_v2_long;
#endif

#ifdef WIN32
#pragma pack()
#endif
