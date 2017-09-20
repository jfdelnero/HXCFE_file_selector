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

#include "gui_utils.h"

#include "cfg_file.h"

#include "hardware.h"

#include "conf.h"
#include "ui_context.h"

#include "menu.h"

extern uint16_t SCREEN_XRESOL;

int enter_menu(ui_context * uicontext, const menu * submenu)
{
	int i,item_count,max_len,t;
	int cb_return;
	unsigned char c;

	max_len = 0;
	i = 0;
	while( submenu[i].text )
	{
		t = strlen((char*)submenu[i].text);

		if(max_len<t)
			max_len = t;

		i++;
	}

	// Center & align the parameters if possible...
	if( max_len*8 < SCREEN_XRESOL/2 )
		max_len = (SCREEN_XRESOL/8)/2;

	i = 0;
	while( submenu[i].text )
	{
		hxc_print(submenu[i].align,0,FILELIST_Y_POS+(i*8), (char*)submenu[i].text);

		if(submenu[i].menu_cb)
		{
			submenu[i].menu_cb(uicontext,0,max_len*8,FILELIST_Y_POS+(i*8),submenu[i].cb_parameter);
		}
		i++;
	}

	item_count = i;

	i = 0;

	invert_line(0,FILELIST_Y_POS+(i*8));

	do
	{
		cb_return = MENU_STAYINMENU;

		c=wait_function_key();
		switch(c)
		{
			case FCT_UP_KEY:
				invert_line(0,FILELIST_Y_POS+(i*8));
				if(i)
					i--;
				invert_line(0,FILELIST_Y_POS+(i*8));
			break;
			case FCT_DOWN_KEY:
				invert_line(0,FILELIST_Y_POS+(i*8));
				if( i < item_count - 1 )
					i++;
				invert_line(0,FILELIST_Y_POS+(i*8));
			break;

			case FCT_SELECT_FILE_DRIVEA:
			case FCT_LEFT_KEY:
			case FCT_RIGHT_KEY:
				// call callback.
				invert_line(0,FILELIST_Y_POS+(i*8));
				if(submenu[i].menu_cb)
				{
					cb_return = submenu[i].menu_cb(uicontext,c,max_len*8,FILELIST_Y_POS+(i*8),submenu[i].cb_parameter);
				}
				invert_line(0,FILELIST_Y_POS+(i*8));
			break;
		}
	}while( (( c != FCT_SELECT_FILE_DRIVEA ) || (submenu[i].submenu != (struct menu *)-1)) && (cb_return != MENU_LEAVEMENU) );

	return 0;
}