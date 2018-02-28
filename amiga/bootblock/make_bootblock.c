/*
//
// Copyright (C) 2009-2018 Jean-François DEL NERO
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

// Amiga Boot block builder (Construct Header + sector list to load + Checksum calculation)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "params.h"

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

typedef struct _bblock
{
	unsigned char dos_sign[3];  // "DOS"
	uint8_t flag;
	uint32_t checksum;
	uint32_t rootblock;
	unsigned char code[1024-(3+1+4+4)];
}__attribute__ ((packed)) bblock;

int memsearch(unsigned char * tofind,int sizetofind, unsigned char * buffer,int bufsize )
{
	int i,buf_i;

	buf_i = 0;
	i = 0;
	while( i < sizetofind && (buf_i+i)<bufsize)
	{
		if(buffer[buf_i+i] != tofind[i])
		{
			buf_i++;
			i = 0;
		}
		else
		{
			i++;
		}
	}

	if(i == sizetofind)
	{
		return buf_i;
	}

	return -1;
}

int checkexec(unsigned char * exec,int execsize, unsigned char * adf,int adfsize, params *paramszone)
{
	int exec_offset,blocknb,first_offset,last_offset;
	int block,blocksize,blockcnt,ldtindex;

	blocknb = execsize / (512-24);

	if( blocknb * (512-24) < execsize )
		blocknb++;

	paramszone->total_blocks_size = ENDIAN_32BIT(blocknb*512);

	ldtindex = 0;
	blocksize = 512-24;
	blockcnt = 0;
	block = 0;
	first_offset = memsearch(&exec[block * (512-24)],blocksize,adf,adfsize );
	last_offset = first_offset;

	if(first_offset>0)
	{
		blockcnt++;
		block++;

		while( block < blocknb )
		{

			if( execsize - (block * (512-24)) < 512-24 )
			{
				blocksize = execsize - (block * (512-24));
			}
			else
			{
				blocksize = 512-24;
			}

			exec_offset = memsearch(&exec[block * (512-24)],blocksize,adf,adfsize );

			if((exec_offset/512) - (last_offset / 512)!=1)
			{
				// Non contiguous
				if(ldtindex<8)
				{
					paramszone->blocktable[ldtindex].startoffset = ENDIAN_32BIT( first_offset & ~0x1FF );
					paramszone->blocktable[ldtindex].lenght = ENDIAN_32BIT( blockcnt * 512 );
				}
				else
				{
					printf("Loading table is full !\n");
					return -1;
				}

				ldtindex++;
				printf("New table entry : offset:%d  Size:%d\n",first_offset,blockcnt);
				first_offset = exec_offset;
				blockcnt=0;
			}

			blockcnt++;
			last_offset = exec_offset;
			block++;
		}

		if(ldtindex<8)
		{
			paramszone->blocktable[ldtindex].startoffset = ENDIAN_32BIT( first_offset & ~0x1FF );
			paramszone->blocktable[ldtindex].lenght = ENDIAN_32BIT(blockcnt * 512);
		}
		else
		{
			printf("Load table full !\n");
			return -1;
		}
		printf("New table entry : offset:%d  Size:%d\n",first_offset,blockcnt);
	}
	return exec_offset;
}

unsigned char * loadfile(char *filename,void * destptr, int * size)
{
	int i,code_size, maxsize;
	FILE * f_in;
	unsigned char * execfile_data;

	f_in = fopen(filename,"r+b");
	if(f_in)
	{
		fseek(f_in,0,SEEK_END);
		code_size = ftell(f_in);
		fseek(f_in,0,SEEK_SET);

		printf("Reading %s... %d byte(s)\n",filename,code_size);
		if(code_size  <= *size )
		{
			if(!destptr) // Need to alloc the buffer ?
			{
				destptr = malloc(code_size);
			}

			if(destptr)
			{
				fread(destptr,code_size,1,f_in);
				*size = code_size;
			}
			else
			{
				printf("Allocation Error ! %d bytes.\n",code_size);
				fclose(f_in);
				return destptr;
			}
			fclose(f_in);
		}
		else
		{
			printf("Error ! No enough space for this code block !!!\n");
			fclose(f_in);
			destptr = 0;
		}
	}
	else
	{
		printf("Error ! Can't open %s !!!\n",filename);
		destptr = 0;
	}

	return destptr;
}

uint32_t payload_checksum(uint32_t * read_ptr,int size)
{
	int i;
	uint32_t checksum,data;
	uint32_t * last_ptr;

	checksum = 0;
	last_ptr = read_ptr + size;
	while( read_ptr != last_ptr )
	{
		data = *read_ptr++;
		checksum += ENDIAN_32BIT(data);
	}

	return ENDIAN_32BIT(checksum);
}

int main (int argc, char *argv[])
{
	int i,code_size,size,exec_pos;
	FILE * f_out;
	unsigned char * execfile_data, *adf_file;
	int execfile_size;
	uint32_t * ptr;
	uint8_t * ptr8b;
	uint32_t checksum,d;
	params *paramszone;
	int tableoffset;

	bblock BOOTBLOCK;

	printf("Amiga Boot block builder v0.2\n");

	if(argc == 4 )
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
		size = sizeof(BOOTBLOCK.code);
		if(!loadfile(argv[1],&BOOTBLOCK.code, &size))
		{
			printf("Error during loading %s !\n",argv[1]);
			exit(-1);
		}

		// Reading the exec file
		execfile_size = 800*1024;		// Maxsize
		execfile_data = loadfile(argv[3],0, &execfile_size);
		if(!execfile_data)
		{
			printf("Error during loading %s !\n",argv[3]);
			exit(-1);
		}

		size = 512*11*2*80;
		adf_file = loadfile(argv[2],0, &size);
		if(!adf_file)
		{
			printf("Error during loading %s !\n",argv[2]);
			exit(-1);
		}

		tableoffset = memsearch("<HXCFEM>",2*4, (unsigned char*)&BOOTBLOCK, sizeof(BOOTBLOCK));
		if(tableoffset<0)
		{
			printf("Entry table not found !!\n");
			free(execfile_data);
			free(adf_file);
			exit(-1);
		}

		ptr8b = (uint8_t *)&BOOTBLOCK;
		paramszone = (params *)&ptr8b[tableoffset + (2*4)];
		exec_pos = checkexec(execfile_data,execfile_size, adf_file,size,paramszone);
		if(exec_pos>0)
		{
			printf("Exec found at offset %d !\n",exec_pos);
		}
		else
		{
			printf("Exec Not found in the ADF file !\n");
			free(execfile_data);
			free(adf_file);
			exit(-1);
		}

		paramszone->exec_size = ENDIAN_32BIT(execfile_size);

		// Compute payload checksum...
		paramszone->exec_checksum = payload_checksum( (uint32_t *)execfile_data,execfile_size>>2);

		printf("Payload checksum : 0x%.8X\n", ENDIAN_32BIT(paramszone->exec_checksum));

		// Compute bootblock checksum...
		ptr = (uint32_t *)&BOOTBLOCK;
		checksum = 0x00000000;
		for( i = 0 ; i < sizeof(BOOTBLOCK) / sizeof(uint32_t) ; i++ )
		{
			d = ENDIAN_32BIT( ptr[i] );

			if( (checksum + d) < checksum )
			{
				checksum++;
			}

			checksum += d;
		}
		checksum = ~checksum;

		BOOTBLOCK.checksum = ENDIAN_32BIT(checksum);

		printf("Bootblock checksum : 0x%.8X\n",checksum);

		printf("Writing to %s...\n",argv[2]);
		f_out = fopen(argv[2],"r+");
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
		printf("Usage : %s [BINCODE] [ADF FILE TO PATCH] [EXECFILE TO LAUNCH]\n",argv[0]);
	}

	return 0;
}
