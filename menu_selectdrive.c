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
#include "ui_context.h"
#include "gui_utils.h"

#include "menu.h"
#include "hal.h"

#include "fectrl.h"

extern unsigned char cfgfile_header[512];

static int selectdrive_menu_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	int drive;

	if(event)
	{
		drive = parameter;
		hxc_printf_box(ctx,"Init emulator I/O...");
		deinit_fdc();
		init_fdc(drive);
		mount_drive(ctx, drive);
	}

	return MENU_LEAVEMENU;
}

const menu selectdrive_menu[]=
{
	{"",                                0,                                0, 0, CENTER_ALIGNED},	
	{"Select Drive:",   0,                                                0, 0, CENTER_ALIGNED},
	{"",                                0,                                0, 0, CENTER_ALIGNED},
	{"A: / DF0",                        selectdrive_menu_cb,              0, 0, CENTER_ALIGNED},
	{"B: / DF1",                        selectdrive_menu_cb,              1, 0, CENTER_ALIGNED},
	{"DF2",                             selectdrive_menu_cb,              2, 0, CENTER_ALIGNED},
	{"DF3",                             selectdrive_menu_cb,              3, 0, CENTER_ALIGNED},
	{"",                                0,                                0, 0, CENTER_ALIGNED},
	{"--- Exit ---",                    0,                                0, (struct menu * )-1, CENTER_ALIGNED},
	{0, 0 , 0 ,0}
};
