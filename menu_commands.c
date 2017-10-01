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

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"

#include "menu.h"
#include "menu_settings.h"
#include "menu_selectdrive.h"

#include "fectrl.h"

#include "hal.h"

#include "errors_def.h"

extern unsigned char cfgfile_header[512];

static int commnand_menu_savereboot_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	int ret;

	if(event)
	{
		if(parameter & 0x01) // Save ?
		{
			ret = ui_save(ctx,-1);
			if( ret != ERR_NO_ERROR )
			{
				error_message_box(ctx, ret);
				return MENU_REDRAWMENU;
			}
		}

		if(parameter & 0x02) // Reboot ?
		{
			ui_reboot(ctx);
		}
		else
		{
			waitsec(1);
		}
	}

	return MENU_REDRAWMENU;
}

static int commnand_menu_chgcolor_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	if(event)
	{
		ui_chgcolor(ctx,ctx->colormode+1);
	}

	return MENU_STAYINMENU;
}

static int commnand_menu_help_cb(ui_context * ctx, int event, int xpos, int ypos, int parameter)
{
	if(event)
	{
		print_help(ctx);
	}
	return MENU_REDRAWMENU;
}

const menu commands_menu[]=
{
	{"--- Save and Settings ---",       0,                      PAGE_FILEBROWSER, (struct menu * )-1, CENTER_ALIGNED},
	{"",                                0,                                     0, 0, CENTER_ALIGNED},
	{"Save and Reboot",                 commnand_menu_savereboot_cb,         0x3, 0, CENTER_ALIGNED},
	{"Save",                            commnand_menu_savereboot_cb,         0x1, 0, CENTER_ALIGNED},
	{"Reboot",                          commnand_menu_savereboot_cb,         0x2, 0, CENTER_ALIGNED},
	{"",                                0,                                     0, 0, CENTER_ALIGNED},
//	{"Quit the File selector",          0,                                    -1, (struct menu * )-1, CENTER_ALIGNED},
//	{"",                                0,                                     0, 0, CENTER_ALIGNED},
	{"Change display colors",           commnand_menu_chgcolor_cb,             0, 0, CENTER_ALIGNED},
	{"HxC Drive Settings",              0,                                     0, (struct menu * )&settings_menu, CENTER_ALIGNED},
	{"",                                0,                                     0, 0, CENTER_ALIGNED},
	{"Select drive",                    0,                                     0, (struct menu * )&selectdrive_menu, CENTER_ALIGNED},
	{"",                                0,                                     0, 0, CENTER_ALIGNED},
	{"Help / About",                    commnand_menu_help_cb,                 0, 0, CENTER_ALIGNED},
	{0, 0 , 0 ,0}
};
