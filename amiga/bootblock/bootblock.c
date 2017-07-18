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

// Bootblock main code.

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <stdint.h>

#include "params.h"

extern struct IOStdReq *ioreq;
extern params Params;

int bootblock_main()
{
	unsigned char * loading_mem;
	int block,i,j,out_offset,in_offset;
	unsigned long checksum;
	int retry;

	params * paramszone;

	retry = 3;
	paramszone = &Params;

	loading_mem = AllocMem(64*1024, MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC);
	if( loading_mem )
	{
		while(retry)
		{
			out_offset = 0;
			block = 0;
			while(paramszone->blocktable[block].lenght)
			{
				ioreq->io_Length = paramszone->blocktable[block].lenght;
				ioreq->io_Data = (APTR)&loading_mem[out_offset];
				ioreq->io_Offset = (ULONG)(paramszone->blocktable[block].startoffset);
				ioreq->io_Command = CMD_READ;
				if(DoIO((struct IORequest *)ioreq))
				{
					retry--;
					break;
				}

				out_offset += paramszone->blocktable[block].lenght;

				block++;
			}

			out_offset = 0;
			in_offset = 0;
			while(out_offset < paramszone->exec_size)
			{
				j = 0;
				while(out_offset < paramszone->exec_size && j<0x1E8)
				{
					loading_mem[out_offset] = loading_mem[in_offset + (j+0x18)];
					out_offset++;
					j++;
				}
				in_offset += 512;
			}

			// Checksum...
			checksum = 0x00000000;
			for( i = 0 ;i < paramszone->exec_size; i++ )
			{
				checksum += loading_mem[i];
			}

			if( paramszone->exec_checksum == checksum )
			{
				// Good !
				for(;;);
			}
			else
			{
				retry--;
			}
		}

		// Error...
		FreeMem(loading_mem, 256*1024);
	}

	// Something wrong happened...
	return 1;
}
