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

#include "conf.h"
#include "ui_context.h"

#include "gui_utils.h"

#include "fectrl.h"

#include "menu.h"

extern unsigned char cfgfile_header[512];

static int settings_menu_stepsound_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		cfgfile_ptr->step_sound =~ cfgfile_ptr->step_sound;
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%s ",cfgfile_ptr->step_sound?"on":"off");

	return MENU_STAYINMENU;
}

static int settings_menu_usersound_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		if(	event == FCT_LEFT_KEY )
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
	}
	if(!cfgfile_ptr->buzzer_duty_cycle)
		cfgfile_ptr->ihm_sound=0x00;

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%d  ",cfgfile_ptr->buzzer_duty_cycle);

	return MENU_STAYINMENU;
}

static int settings_menu_lcdstandby_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		if(	event == FCT_LEFT_KEY )
		{
			if(cfgfile_ptr->back_light_tmr)
				cfgfile_ptr->back_light_tmr--;
		}
		else
		{
			if(cfgfile_ptr->back_light_tmr<0xFF)
				cfgfile_ptr->back_light_tmr++;
		}
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos,"%d s ",cfgfile_ptr->back_light_tmr);

	return MENU_STAYINMENU;
}

static int settings_menu_sdstandby_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		if(	event == FCT_LEFT_KEY )
		{
			if(cfgfile_ptr->standby_tmr)
				cfgfile_ptr->standby_tmr--;
		}
		else
		{
			if(cfgfile_ptr->standby_tmr<0xFF)
				cfgfile_ptr->standby_tmr++;
		}
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%d s ",cfgfile_ptr->standby_tmr);

	return MENU_STAYINMENU;
}

static int settings_menu_driveb_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		cfgfile_ptr->enable_drive_b=~cfgfile_ptr->enable_drive_b;
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%s ",cfgfile_ptr->enable_drive_b?"off":"on");

	return MENU_STAYINMENU;
}

static int settings_menu_autobootpowerup_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		cfgfile_ptr->startup_mode ^= START_MODE_SLOT_0;
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%s ",(cfgfile_ptr->startup_mode & START_MODE_SLOT_0)?"on":"off");

	return MENU_STAYINMENU;
}

static int settings_menu_ejectpowerup_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	cfgfile * cfgfile_ptr;

	cfgfile_ptr=(cfgfile * )cfgfile_header;

	if(event)
	{
		cfgfile_ptr->startup_mode ^= START_MODE_DSKEJECTED;
	}

	hxc_printf(ctx,LEFT_ALIGNED,xpos,ypos, "%s ",(cfgfile_ptr->startup_mode & START_MODE_DSKEJECTED)?"on":"off");

	return MENU_STAYINMENU;
}

const menu settings_menu[]=
{
	{"",                                0,                                0, 0, LEFT_ALIGNED},
	{"HxC Floppy Emulator settings:",   0,                                0, 0, LEFT_ALIGNED},
	{"",                                0,                                0, 0, LEFT_ALIGNED},
	{"Track step sound :",              settings_menu_stepsound_cb,       0, 0, LEFT_ALIGNED},
	{"User interface sound:",           settings_menu_usersound_cb,       0, 0, LEFT_ALIGNED},
	{"",                                0,                                0, 0, LEFT_ALIGNED},	
	{"LCD Backlight standby:",          settings_menu_lcdstandby_cb,      0, 0, LEFT_ALIGNED},
	{"SD/USB Standby:",                 settings_menu_sdstandby_cb,       0, 0, LEFT_ALIGNED},
	{"",                                0,                                0, 0, LEFT_ALIGNED},	
	{"DF1 drive :",                     settings_menu_driveb_cb,          0, 0, LEFT_ALIGNED},
	{"Load AUTOBOOT.HFE at power up :", settings_menu_autobootpowerup_cb, 0, 0, LEFT_ALIGNED},
	{"Eject disk at power up :",        settings_menu_ejectpowerup_cb,    0, 0, LEFT_ALIGNED},
	{"",                                0,                                0, 0, LEFT_ALIGNED},
	{"--- Exit ---",                    0,                                0, (struct menu * )-1, CENTER_ALIGNED},
	{0, 0 , 0 ,0}
};
