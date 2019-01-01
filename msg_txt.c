/*
//
// Copyright (C) 2009-2019 Jean-François DEL NERO
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

#include "version.h"

#include "cfg_file.h"
#include "ui_context.h"
#include "gui_utils.h"
#include "msg_txt.h"

const char startup_msg[]=
{
	"HxC Floppy Emulator file selector\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"\n"
	"(c) 2009-2019 HxC2001 / Jean-Francois DEL NERO\n"
	"\n"
	"Email : hxc2001@free.fr\n"
	"\n"
	">>> Press HELP key for the function key list <<<"
	"\n"
	"Updates:  http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"          https://hxc2001.com\n"
	"Forum:    http://torlus.com/floppy/forum\n"
	"Facebook: https://www.facebook.com/groups/hxc2001\n"
	"Sources : http://github.com/jfdelnero\n"
	"\n"
	"This software uses the FAT16/32 File IO Library v2.6\n"
	"(c) 2003-2013 Ultra-Embedded.com\n"
};

const char help_scr1_msg[]=
{
	"Function Keys (1/3):\n"
	"\n"
	"-> Into the files browser :\n"
	"\n"
	"Keyboard Arrows and Joystick  : Browse the SD/USB files\n"
	"Right Shift                   : Go back to the top of the folder\n"
	"F7                            : Insert the selected file in the slot 1\n"
    "                                and restart the computer with this disk.\n"
	"ENTER/Joystick Fire           : Enter a subfolder/ Select an image\n"
	"                                and enter the slots selection\n"
	"\n"
	"\n"
	"                   ---Press Enter / Fire to continue---"
};

const char help_scr2_msg[]=
{
	"Function Keys (2/3):\n"
	"\n"
	"-> Into the slots browser :\n"
	"\n"
	"Keyboard Arrows and Joystick  : Browse the slots selection\n"
	"BACKSPACE                     : Clear the current slot\n"
	"ENTER/Joystick Fire           : Set the current slot\n"
	"                                and leave the slots browser\n"
	"ESCAPE                        : Leave the slots browser\n"
	"\n"
	"\n"
	"\n"
	"                   ---Press Enter / Fire to continue---"
};

const char help_scr3_msg[]=
{
	"Function Keys (3/3):\n"
	"\n"
	"F1                : Search files in the current folder\n"
	"                    Type the word to search, then enter\n"
	"                    Escape to abort the search\n"
	"F2                : Change color\n"
	"F3                : Settings menu\n"
	"F8                : Reboot\n"
	"F9                : Save\n"
	"F10               : Save and Reboot\n"
	"\n"
	"\n"
	"                   ---Press Enter / Fire to continue---"
};

const char help_scr4_msg[]=
{
	"Notes :\n"
	"\n"
	"The joystick can be used to browse all the interface and select the images.\n"
	"The keyboard is not required.\n"
	"\n"
	"The first line of each page allows to switch to the next one :\n"
	"Media files list ->Drive A slots list ->Drive B slots list ->Save/Settings page\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"                   ---Press Enter / Fire to continue---"
};

const char help_scr5_msg[]=
{
	"HxC Floppy Emulator file selector\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"\n"
	"(c) 2006-2019 HxC2001 / Jean-Francois DEL NERO\n"
	"Email : hxc2001@free.fr\n"
	"\n"
	"Atari ST version optimized by Gilles Bouthenot\n"
	"\n"
	"This software uses the FAT16/32 File IO Library\n"
	"(c) 2003-2013 Ultra-Embedded.com\n"
	"\n"
	"\n"
	"---Press Enter / Fire to continue---"
};

const char help_scr6_msg[]=
{
	"HxC Floppy Emulator file selector\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"\n"
	"(c) 2006-2019 HxC2001 / Jean-Francois DEL NERO\n"
	"Email : hxc2001@free.fr\n"
	"\n"
	"Updates : http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"          https://hxc2001.com                          \n"
	"Forum   : http://torlus.com/floppy/forum               \n"
	"Facebook: https://www.facebook.com/groups/hxc2001      \n"
	"Sources : https://github.com/jfdelnero                 \n"
	"\n"
	"---Press Enter / Fire to continue---"
};

const pagedesc help_pages[]=
{
	{help_scr1_msg,LEFT_ALIGNED},
	{help_scr2_msg,LEFT_ALIGNED},
	{help_scr3_msg,LEFT_ALIGNED},
	{help_scr4_msg,LEFT_ALIGNED},
	{help_scr5_msg,CENTER_ALIGNED},
	{help_scr6_msg,CENTER_ALIGNED},
	{0,0}
};

const char cur_folder_msg[] = "Current folder:";
const char reboot_msg[] = ">>>>>Rebooting...<<<<<";
const char save_msg[] = "Saving selection...";
const char save_and_restart_msg[] = "Saving selection and restarting...";
const char title_msg[] = { "HxC Floppy Emulator file selector v" VERSIONCODE};
const char copyright_msg[] = "(c)HxC2001";

const char need_update_msg[] =
{
	"\n"
	"\n"
	"Your firmware version appears to be outdated !\n\n"
	"Please visit the HxC2001 download page to get\n"
	"the latest firmware version and update your emulator !\n\n"
	"https://hxc2001.com/download/floppy_drive_emulator/\n"
	"\n"
	"\n"
	"Contact me at hxc2001@hxc2001.com if you got any issue !\n"
	"\n"
	"---Press Enter / Fire to continue---"
};

const char project_support_msg[]=
{
	"\n"
	"\n"
	"You are using a third party firmware based on the HxC protocol.\n"
	"Please consider supporting the HxC2001 project if you like this\n"
	"file selector and all the tools and file formats originally\n"
	"developed by/for the HxC2001 Project.\n"
	"\n"
	"To support the project : https://hxc2001.com/store\n"
	"\n"
	"Thanks in advance ! :)\n"
	"Jean-Francois DEL NERO\n"
	"\n"
	"---Press Enter / Fire to continue---"
};
