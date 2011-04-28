/*
//
// Copyright (C) 2009, 2010, 2011 Jean-François DEL NERO
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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <delays.h>
#include <usart.h>
#include <stdarg.h>

#include "fat32\browse.h"
#include "fat32\fat.h"
#include "hardware.h"
#include "cfg_file.h"
#include "utils.h"

extern unsigned char step_sound_cfg;	// 0xF2B 
extern unsigned char ihm_sound_cfg;	// 0xF2C
extern unsigned char backlight_tmr_cfg; //0xF2D
extern unsigned char standby_tmr_cfg;	//0xF2E
extern unsigned char buzzer_duty_cycle_cfg; //0xF30

extern unsigned char sectorBuffer[0x200];
extern unsigned char number_of_slot;
extern unsigned char slot_index;
extern unsigned char disable_drive_select;


unsigned char get_device_parameters(struct DirectoryEntry *pDirEnt)
{
	unsigned char i;
	cfgfile * cfgf;

	readfile(pDirEnt,0,0xc,0xd);
	cfgf=(cfgfile *)&sectorBuffer;

	if(!strcmppgm2ram ((char*)cfgf->signature,(const far rom char *)"HXCFECFGV1.0"))
	{
		step_sound_cfg=cfgf->step_sound;	// 0xF2B 
		ihm_sound_cfg=cfgf->ihm_sound;	// 0xF2C
		backlight_tmr_cfg=cfgf->back_light_tmr; //0xF2D
		standby_tmr_cfg=cfgf->standby_tmr;	//0xF2E
		buzzer_duty_cycle_cfg=cfgf->buzzer_duty_cycle;
		number_of_slot=cfgf->number_of_slot;
		slot_index=cfgf->slot_index;
		disable_drive_select=cfgf->disable_drive_select;
		return 0xFF;
	}
	return 0;
}

void update_cfgfile_parameters(struct DirectoryEntry *pDirEnt,unsigned char slot_index)
{
	cfgfile * cfgf;

	readfile(pDirEnt,0,0xc,0xd);

	cfgf=(cfgfile *)&sectorBuffer;
	cfgf->update_cnt++;
	cfgf->slot_index=slot_index;

	writefile(pDirEnt,0,0xc,0xd);

	return;
}
