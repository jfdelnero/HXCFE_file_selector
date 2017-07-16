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

// Amiga Boot block builder (Construct Header + Checksum calculation)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HOST_IS_BIG_ENDIAN
	// Big Endian host - as the 68k... nothing to do.
	#define ENDIAN_32BIT(value)	value
	#define ENDIAN_16BIT(value)	value
#else
	// Little Endian
	#define ENDIAN_32BIT(value)	( ((uint32_t)(value&0x000000FF)<<24) + \
	                              ((uint32_t)(value&0x0000FF00)<<8) + \
								  ((uint32_t)(value&0x00FF0000)>>8) + \
								  ((uint32_t)(value&0xFF000000)>>24) )

	#define ENDIAN_16BIT(value)	( ((uint16_t)(value&0x00FF)<<8) + ((uint16_t)(value&0xFF00)>>8)  )
#endif

#pragma pack(1)
typedef struct _bblock
{
	unsigned char dos_sign[3];  // "DOS"
	uint8_t flag;
	uint32_t checksum;
	uint32_t rootblock;
	unsigned char code[1024-(3+1+4+4)];
}bblock;
#pragma pack()

int main (int argc, char *argv[])
{
	int i,code_size;
	FILE * f_out;
	FILE * f_in;
	uint32_t * ptr;
	uint32_t checksum,d;

	bblock BOOTBLOCK;

	printf("Amiga Boot block builder v0.1\n");

	if(argc == 3 )
	{
		memset((void*)&BOOTBLOCK,0,sizeof(BOOTBLOCK));
		strcpy(BOOTBLOCK.dos_sign,"DOS");
		/*
		flags = 3 least signifiant bits
			set         clr
		0   FFS         OFS
		1   INTL ONLY   NO_INTL ONLY
		2   DIRC&INTL   NO_DIRC&INTL
		OFS = Old/Original File System, the first one. (AmigaDOS 1.2)
		FFS = Fast File System (AmigaDOS 2.04)
		INTL = International characters Mode.
		DIRC = stands for Directory Cache Mode. This mode speeds up
		directory listing, but uses more disk space
		*/

		BOOTBLOCK.flag = 0x00; //OFS only.
		BOOTBLOCK.rootblock = ENDIAN_32BIT(880);
		BOOTBLOCK.checksum = 0x00000000;

		// Reading block code
		f_in = fopen(argv[1],"rb");
		if(f_in)
		{
			fseek(f_in,0,SEEK_END);
			code_size = ftell(f_in);
			fseek(f_in,0,SEEK_SET);
			printf("Reading %s... %d byte(s)\n",argv[2],code_size);
			if(code_size < sizeof(BOOTBLOCK.code))
			{
				fread(&BOOTBLOCK.code,code_size,1,f_in);
				fclose(f_in);
			}
			else
			{
				printf("Error ! No enough space for this code block !!!\n");
				fclose(f_in);
				exit(-1);
			}
		}
		else
		{
			printf("Error ! Can't open %s !!!\n",argv[2]);
			exit(-1);
		}

		// Compute checksum...
		ptr = (uint32_t *)&BOOTBLOCK;
		checksum = 0x00000000;
		for( i = 0 ; i < sizeof(BOOTBLOCK)/4 ; i++ )
		{
			d = ENDIAN_32BIT(ptr[i]);

			if ( ( 0xFFFFFFFF - checksum ) < d )	// overflow ?
			{
				checksum++;
			}

			checksum += d;
		}
		checksum ^= 0xFFFFFFFF;

		BOOTBLOCK.checksum = ENDIAN_32BIT(checksum);

		printf("Block checksum : 0x%.8X\n",checksum);

		printf("Writing %s...\n",argv[2]);
		f_out = fopen(argv[2],"wb");
		if(f_out)
		{
			fwrite(&BOOTBLOCK,sizeof(BOOTBLOCK),1,f_out);
			fclose(f_out);
		}
		else
		{
			printf("Error ! Can't create the output file !\n");
			exit(-1);
		}
	}
	else
	{
		printf("Usage : %s [BINCODE] [OUTPUTFILE.BOOT]\n",argv[0]);
	}

	return 0;
}