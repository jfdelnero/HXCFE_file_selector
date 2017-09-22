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

#include "conf.h"

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"

#include "hal.h"

#include "fectrl.h"
#include "menu.h"

int menu_draw(ui_context * ctx, const menu * submenu, int *max_len)
{
	int i,t;

	clear_list(ctx);

	*max_len = 0;
	i = 0;
	while( submenu[i].text )
	{
		t = strlen((char*)submenu[i].text);

		if(*max_len<t)
			*max_len = t;

		i++;
	}

	// Center & align the parameters if possible...
	if( *max_len < ctx->screen_txt_xsize / 2 )
		*max_len = ctx->screen_txt_xsize / 2;

	i = 0;
	while( submenu[i].text )
	{
		hxc_print(ctx,submenu[i].align,0,FILELIST_Y_POS + i, (char*)submenu[i].text);

		if(submenu[i].menu_cb)
		{
			submenu[i].menu_cb(ctx,0,*max_len,FILELIST_Y_POS + i,submenu[i].cb_parameter);
		}
		i++;
	}

	return i;
}

int enter_menu(ui_context * ctx, const menu * submenu)
{
	int i,item_count,max_len;
	int cb_return;
	unsigned char c;

	item_count = menu_draw(ctx, submenu, &max_len);

	i = 0;

	invert_line(ctx, FILELIST_Y_POS + i);

	do
	{
		cb_return = MENU_STAYINMENU;

		c=wait_function_key();
		switch(c)
		{
			case FCT_UP_KEY:
				invert_line( ctx, FILELIST_Y_POS + i );
				if(i)
					i--;
				invert_line( ctx, FILELIST_Y_POS + i );
			break;
			case FCT_DOWN_KEY:
				invert_line( ctx, FILELIST_Y_POS + i );
				if( i < item_count - 1 )
					i++;
				invert_line( ctx, FILELIST_Y_POS + i );
			break;

			case FCT_SELECT_FILE_DRIVEA:
			case FCT_LEFT_KEY:
			case FCT_RIGHT_KEY:
				// call callback.
				invert_line( ctx, FILELIST_Y_POS + i );
				if(submenu[i].menu_cb)
				{
					cb_return = submenu[i].menu_cb(ctx,c,max_len,FILELIST_Y_POS + i,submenu[i].cb_parameter);
					if(cb_return == MENU_REDRAWMENU)
					{
						menu_draw(ctx, submenu, &max_len);
					}
				}

				if(c == FCT_SELECT_FILE_DRIVEA)
				{
					if(submenu[i].submenu && submenu[i].submenu != (struct menu *)-1)
					{
						enter_menu(ctx, (menu *)submenu[i].submenu);
						menu_draw(ctx, submenu, &max_len);
					}
				}

				invert_line( ctx, FILELIST_Y_POS + i );

			break;
		}
	}while( (( c != FCT_SELECT_FILE_DRIVEA ) || (submenu[i].submenu != (struct menu *)-1)) && (cb_return != MENU_LEAVEMENU) );

	return (int)submenu[i].cb_parameter;
}