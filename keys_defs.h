/*
//
// Copyright (C) 2009-2016 Jean-François DEL NERO
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

typedef struct keyboard_funct_mapper_
{
	unsigned char function_code;
	unsigned char keyboard_code;
}keyboard_funct_mapper;

keyboard_funct_mapper keysmap[]=
{
	{FCT_UP_KEY,0x4C},
	{FCT_DOWN_KEY,0x4D},
	{FCT_LEFT_KEY,0x4F},
	{FCT_RIGHT_KEY,0x4E},
	{FCT_SELECT_FILE_DRIVEA,0x66},
	{FCT_SELECT_FILE_DRIVEA,0x44},
	{FCT_SELECT_FILE_DRIVEB,0x67},
	{FCT_SELECT_FILE_DRIVEA_AND_NEXTSLOT,0x64},
	{FCT_NEXTSLOT,0x65},
	{FCT_SAVE,0x58},
	{FCT_REBOOT,0x57},
	{FCT_CLEARSLOT,0x41},
	{FCT_CLEARSLOT_AND_NEXTSLOT,0x46},
	{FCT_HELP,0x5F},
	{FCT_SAVEREBOOT,0x59},
	{FCT_SELECTSAVEREBOOT,0x56},
	{FCT_OK,0x40},
	{FCT_SHOWSLOTS,0x42},
	{FCT_SEARCH,0x50},
	{FCT_TOP,0x61},
	{FCT_CHGCOLOR,0x51},
	{FCT_EMUCFG,0x52},
	{FCT_NO_FUNCTION,0x00}
};

typedef struct keyboard_scancode_mapper_
{
	unsigned char char_code;
	unsigned char keyboard_code;
}keyboard_scancode_mapper;

keyboard_funct_mapper char_keysmap[]=
{
	{'a',0x20},
	{'b',0x35},
	{'c',0x33},
	{'d',0x22},
	{'e',0x12},
	{'f',0x23},
	{'g',0x24},
	{'h',0x25},
	{'i',0x17},
	{'j',0x26},
	{'k',0x27},
	{'l',0x28},
	{'m',0x37},
	{'n',0x36},
	{'o',0x18},
	{'p',0x19},
	{'q',0x10},
	{'r',0x13},
	{'s',0x21},
	{'t',0x14},
	{'u',0x16},
	{'v',0x34},
	{'w',0x11},
	{'x',0x32},
	{'y',0x15},
	{'z',0x31},

	{'0',0x0A},
	{'1',0x01},
	{'2',0x02},
	{'3',0x03},
	{'4',0x04},
	{'5',0x05},
	{'6',0x06},
	{'7',0x07},
	{'8',0x08},
	{'9',0x09},

	//{'0',0x00},
	{'1',0x1D},
	{'2',0x1E},
	{'3',0x1F},
	{'4',0x2D},
	{'5',0x2E},
	{'6',0x2F},
	{'7',0x3D},
	{'8',0x3E},
	{'9',0x3F},
	{' ',0x40},
	{'.',0x39},
	{'\n',0x44},

	{0xFF,0x00},
};
