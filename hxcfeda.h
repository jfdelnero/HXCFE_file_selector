/*
//
// Copyright (C) 2009-2023 Jean-Fran�ois DEL NERO
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

#ifdef WIN32
#pragma pack(1)
#endif

typedef struct direct_access_status_sector_
{
	char DAHEADERSIGNATURE[8];
	char FIRMWAREVERSION[12];
	unsigned long lba_base;
	unsigned char cmd_cnt;
	unsigned char read_cnt;
	unsigned char write_cnt;
	unsigned char last_cmd_status;
	unsigned char write_locked;
	unsigned char keys_status;
	unsigned char sd_status;
	unsigned char SD_WP;
	unsigned char SD_CD;
	unsigned char number_of_sector;
	unsigned short current_index;
#ifndef WIN32
}__attribute__((__packed__)) direct_access_status_sector;
#else
}direct_access_status_sector;
#endif

typedef struct direct_access_cmd_sector_
{
	char DAHEADERSIGNATURE[8];
	unsigned char cmd_code;
	unsigned char parameter_0;
	unsigned char parameter_1;
	unsigned char parameter_2;
	unsigned char parameter_3;
	unsigned char parameter_4;
	unsigned char parameter_5;
	unsigned char parameter_6;
	unsigned char parameter_7;
	unsigned char cmd_checksum;
#ifndef WIN32
}__attribute__((__packed__)) direct_access_cmd_sector;
#else
}direct_access_cmd_sector;
#endif

#define LFN_MAX_SIZE 128

typedef struct DirectoryEntry_ {
	unsigned char name[12];
	unsigned char attributes;
	unsigned long firstCluster;
	unsigned long size;
	unsigned char longName[LFN_MAX_SIZE];	// boolean
#ifndef WIN32
}__attribute__((__packed__)) DirectoryEntry;
#else
}DirectoryEntry;
#endif

#ifdef WIN32
#pragma pack()
#endif
