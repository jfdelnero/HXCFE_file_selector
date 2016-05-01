#include "version.h"

const char startup_msg[]=
{
	"HxC Floppy Emulator file selector for Amiga\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"(c) 2009-2016 HxC2001 / Jean-Francois DEL NERO\n"
	"\n"
	"Check for updates on :\n"
	"http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"http://hxc2001.com\n"
	"\n"
	"Forum : http://torlus.com/floppy/forum\n"
	"\n"
    "Email : hxc2001@free.fr\n"
	"\n"
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
	"Up/Down/Right/Left: Browse the SD/USB files\n"
	"Right Shift       : Go back to the top of the folder\n"
	"F7                : Insert the selected file in the slot to 1 and restart the\n"
    "                    computer with this disk.\n"
	"ENTER             : Enter a subfolder/ Select an Image and Enter the slots\n"
	"                    selection\n"
	"\n"
	"\n"
	"->Into the slots browser :\n"
	"\n"
	"Up/Down/Right/Left: Browse the slots selection\n"
	"BACKSPACE         : Clear the current slot\n"
	"ENTER             : Set the current slot and leave the slots browser\n"
	"ESCAPE            : Leave the slots browser\n"
	"\n"
	"\n"
	"\n"
	"                        ---Press Space to continue---"
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
	"\n"
};

const char help_scr3_msg[]=
{
	"---Press Space to exit---\n"
	"\n"
	"\n"
	"HxC Floppy Emulator file selector for Amiga\n"
	"V" VERSIONCODE " - " DATECODE "\n"
	"(c) 2006-2016 HxC2001 / Jean-Francois DEL NERO\n"
	"\n"
	"Check for updates on :\n"
	"http://hxc2001.free.fr/floppy_drive_emulator/\n"
	"http://hxc2001.com\n"
	"\n"
	"Forum : http://torlus.com/floppy/forum\n"
	"Email : hxc2001@free.fr\n"
};

const char command_menu_msg[]=
{
	"--- Commands ---\n"
	"\n"
	"Save And Reboot\n"
	"Save\n"
	"Reboot\n"
	"\n"
	"Change colors\n"
	"Drive Settings\n"
};

const char cur_folder_msg[]= "Current folder:";
const char reboot_msg[]= ">>>>>Rebooting...<<<<<";
const char title_msg[]= { "Amiga HxC Floppy Emulator Manager v" VERSIONCODE};
