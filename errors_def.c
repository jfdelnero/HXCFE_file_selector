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

#include <stdint.h>

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"

#include "hal.h"

#include "errors_def.h"

typedef struct _err_mess_txt
{
	int error_code;
	const char * text;
}err_mess_txt;

err_mess_txt error_messages[]=
{
	{ ERR_NO_ERROR,                       "ERROR: No Error"},
	{ ERR_MEDIA_READ,                     "ERROR: General READ Error !"},
	{ ERR_MEDIA_READ_SECTOR_NOT_FOUND,    "ERROR: Read - Sector not found !"},
	{ ERR_MEDIA_READ_NO_INDEX,            "ERROR: READ - No Index Timeout !"},
	{ ERR_MEDIA_WRITE,                    "ERROR: General WRITE Error !"},
	{ ERR_MEDIA_WRITE_SECTOR_NOT_FOUND,   "ERROR: WRITE - Sector not found !"},
	{ ERR_MEDIA_WRITE_NO_INDEX,           "ERROR: WRITE - No Index Timeout !"},
	{ ERR_MEM_ALLOC,                      "ERROR: Memory Allocation Error !"},
	{ ERR_SYSLIB_LOAD,                    "ERROR: System lib access Error !"},
	{ ERR_DRIVE_NOT_FOUND,                "ERROR: No HxC Drive Found !"},
	{ ERR_TRACK0_SEEK,                    "ERROR: writesector -> failure while seeking the track 00!"},
	{ ERR_INVALID_PARAMETER,              "ERROR: Invalid function parameter(s)."},
	{ ERR_MEDIA_ATTACH,                   "ERROR: Media attach failed !"},
	{ ERR_LBA_CHANGE_FAILURE,             "ERROR: LBA Change Test Failed ! Write Issue ?"},
	{ ERR_BAD_DRIVE_ID,                   "ERROR: Bad signature - HxC Floppy Emulator not found!"},
	{ ERR_CONFIG_FILE_ACCESS,             "ERROR: Can't open HXCSDFE.CFG !"},
	{ ERR_CONFIG_FILE_VERSION,            "ERROR: Bad Config File version !"},
	{ ERR_CONFIG_FILE_SIGN,               "ERROR: Bad Config File !"},
	{ ERR_READ_FILE_ACCESS,               "ERROR: Read file failed!"},
	{ ERR_WRITE_FILE_ACCESS,              "ERROR: Write file failed!"},
	{ ERR_INVALID_HANDLER,                "ERROR: Invalid Handler!"},
	{ 0, 0}
};

void error_message_box(ui_context * ctx, int error_code)
{
	int i;

	i = 0;
	while(error_messages[i].text)
	{
		if( (error_messages[i].error_code) == -error_code )
		{
			hxc_printf_box(ctx, (char*)error_messages[i].text);

			waitsec(5);

			return;
		}
		i++;
	}

	hxc_printf_box(ctx, "ERROR: Unknown Error ! [%d]",error_code);

	waitsec(5);
}
