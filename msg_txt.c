#include "version.h"

const char startup_msg[]=
{
	"Amiga HxC Floppy Emulator file selector\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"(c) 2009-2016 HxC2001 / Jean-Francois DEL NERO\n"
	"Email : hxc2001@free.fr\n"
	"\n"
	"Check for updates on :\n"
	"http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"http://hxc2001.com\n"
	"\n"
	"Forum : http://torlus.com/floppy/forum\n"
	"\n"
	"Sources on http://github.com/jfdelnero\n"
	"\n"
	"\n"
	"\n"
	">>> Press HELP key for the function key list <<<"
};

const char help_scr1_msg[]=
{
	"Function Keys (1/2):\n"
	"\n"
	"-> Into the files browser :\n"
	"\n"
	"Keyboard Arrows and Joystick : Browse the SD/USB files\n"
	"Right Shift                  : Go back to the top of the folder\n"
	"F7                           : Insert the selected file in the slot 1\n"
    "                               and restart the computer with this disk.\n"
	"ENTER/Joystick Fire          : Enter a subfolder/ Select an image\n"
	"                               and enter the slots selection\n"
	"\n"
	"\n"
	"->Into the slots browser :\n"
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

const char help_scr2_msg[]=
{
	"Function Keys (2/2):\n"
	"\n"
	"F1                : Search files in the current folder\n"
	"                    Type the word to search then enter\n"
	"                    Excape to abord the search\n"
	"F2                : Change color\n"
	"F3                : Settings menu\n"
	"F8                : Reboot\n"
	"F9                : Save\n"
	"F10               : Save and Reboot\n"
	"\n"
	"The joystick can be used to browse all the interface and select the images.\n"
	"The keyboard is not required.\n"
	"The first line of each page allows to switch to the next one :\n"
	"Media files list-> Drive A slots list-> Drive B slots list-> Save/Settings page\n"
};

const char help_scr3_msg[]=
{
	"---Press Enter / Fire to exit---\n"
	"\n"
	"Amiga HxC Floppy Emulator file selector\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"(c) 2006-2016 HxC2001 / Jean-Francois DEL NERO\n"
	"Email : hxc2001@free.fr\n"
	"\n"
	"Check for updates on :\n"
	"http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"http://hxc2001.com\n"
	"\n"
	"Forum : http://torlus.com/floppy/forum\n"
	"Sources on http://github.com/jfdelnero\n"
};

const char command_menu_msg[]=
{
	"--- Save and Settings ---\n"
	"\n"
	"Save and Reboot\n"
	"Save\n"
	"Reboot\n"
	"\n"
	"Change display colors\n"
	"HxC Drive Settings\n"
	"\n"
	"Help / About\n"
};

const char cur_folder_msg[]= "Current folder:";
const char reboot_msg[]= ">>>>>Rebooting...<<<<<";
const char title_msg[]= { "Amiga HxC Floppy Emulator Manager v" VERSIONCODE};
