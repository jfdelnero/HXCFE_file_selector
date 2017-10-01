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

#ifdef WIN32
#pragma pack(1)
#endif

typedef struct Cortex_cfgfile_
{
	uint8_t  undef[22];
	uint16_t number_of_slot;
	uint16_t slot_index;
	uint16_t update_cnt; 
#ifndef WIN32
}__attribute__((__packed__)) Cortex_cfgfile;
#else
}Cortex_cfgfile;
#endif

struct Cortex_ShortDirectoryEntry {
	unsigned char name[12];
    uint8_t  attributes;
    uint32_t firstCluster;
    uint32_t size;
	unsigned char longName[41];	// boolean
#ifndef WIN32
}__attribute__((__packed__));
#else
};
#endif

extern struct Cortex_DirectoryEntry Cortex_directoryEntry;

typedef struct Cortex_disk_in_drive_
{
	struct Cortex_ShortDirectoryEntry DirEnt;
#ifndef WIN32
}__attribute__((__packed__)) Cortex_disk_in_drive;
#else
}Cortex_disk_in_drive;
#endif

#ifdef WIN32
#pragma pack()
#endif
