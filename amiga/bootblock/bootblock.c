/*
//
// Copyright (C) 2009-2022 Jean-François DEL NERO
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
/////////////////////////////////////////////////////////////////////
//
// Bootblock main code.
//
// This Amiga blocksector trackload from a sector list an executable
// present on the file system, remove the sectors header, check the
// load integrity, alloc and reloc the executable and finally launch it.
//
// Total Bootblock size (Header + Params Area + Code) : 844 Bytes.
// (20 July 2017)
//
/////////////////////////////////////////////////////////////////////

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <dos/doshunks.h>

#include <stdint.h>

#include "jump_to_exec.h"
#include "hunk.h"
#include "params.h"

#define BG_COLOR_ERROR_1     0xFF0 // Yellow
#define BG_COLOR_ERROR_2     0xF80 // Orange
#define BG_COLOR_ERROR_3     0xF00 // Red
#define BG_COLOR_OK          0x070 // Green
#define BG_COLOR_PROGRESS    0x005 // Blue

#define COLOR00_ADDR 0xDFF180

static uint32_t * cpy_hunk_buf(uint32_t * d,uint32_t * s,int size)
{
	while(size--)
		*d++ = *s++;

	return s;
}

static void pack_payload(uint32_t * base_ptr,int size)
{
	int i;
	uint32_t * read_ptr;
	uint32_t * write_ptr;

	write_ptr = base_ptr;
	read_ptr = base_ptr;

	while(write_ptr - base_ptr < size)
	{
		read_ptr += ( 24 / 4 ); // Skip Sector header
		i=0;
		while( i < ( ( 512-24 ) / 4 ) ) // Move the 488 bytes.
		{
			*write_ptr++ = *read_ptr++;
			i++;
		}
	}
}

static uint32_t payload_checksum(uint32_t * read_ptr,int size)
{
	uint32_t checksum;
	uint32_t * last_ptr;

	checksum = 0;
	last_ptr = read_ptr + size;
	while( read_ptr != last_ptr )
	{
		checksum += *read_ptr++;
	}

	return checksum;
}

// Get the section long offset
static uint32_t get_hunk_section_offset(hunk_header * hunkh, int index)
{
	uint32_t offset;
	int i;

	offset = 0;
	for (i = 0; i < index; i++)
	{
		offset += hunkh->HUNKSIZE[i];
	}
	return offset;
}

uint32_t * load_hunk(uint32_t * base_ptr, int size)
{
	uint32_t i;
	uint32_t totalhunk_size;
	uint32_t * read_ptr;
	uint32_t * output_hunk_offset;
	uint32_t * output_buffer;
	uint32_t * pointer_to_patch;
	uint32_t nb_of_offsets;
	uint32_t hunk_id, hunk_id_reloc;
	hunk_header * hunkh;

	hunkh = (hunk_header *)base_ptr;

	if ( hunkh->HUNK_ID == HUNK_HEADER && !hunkh->RESIDENTLIBLIST) // Check Header
	{
		// Compute the needed memory size.
		totalhunk_size = 0;
		for (i = 0; i < hunkh->NB_OF_HUNKS; i++)
		{
			totalhunk_size += hunkh->HUNKSIZE[i];
		}

		// Try to allocate it.
		output_buffer = AllocMem(totalhunk_size<<2, MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC);
		if(!output_buffer)
		{
			// Fatal error...
			return 0;
		}

		// Read pointer right after the hunk size list.
		read_ptr = (uint32_t *)&hunkh->HUNKSIZE[i];

		hunk_id = 0;
		output_hunk_offset = output_buffer;

		while ( (read_ptr - base_ptr) < size )
		{
			switch ( *read_ptr++ ) // HUNK section ID
			{
				case HUNK_CODE:
				case HUNK_DATA:
					// Just copy the Code/Hunk section
					read_ptr = cpy_hunk_buf(output_hunk_offset, read_ptr + 1, *read_ptr);
				break;

				case HUNK_BSS:
					// Here we should clear the BSS, but this is already done by the alloc function.
					read_ptr++; // BBS size.
				break;

				case HUNK_RELOC32:
					while ( *read_ptr )               // New HUNK to reloc ?
					{
						nb_of_offsets = *read_ptr++;  // Get the number of offset to patch.
						hunk_id_reloc = *read_ptr++;  // Get the HUNK id.

						// Patchs the offsets...
						for (i = 0; i < nb_of_offsets; i++)
						{
							// Recast needed for the offset calculation : Addresses to patch may be unaligned
							// Get the position of the address to modify.
							pointer_to_patch = (uint32_t *)( *read_ptr++ + (uint8_t *)output_hunk_offset );

							// Update the opcode address parameter.
							*pointer_to_patch += (uint32_t)(&output_buffer[ get_hunk_section_offset(hunkh, hunk_id_reloc) ]);
						}
					}

					read_ptr++;
				break;

				case HUNK_END:
					// Next Hunk destination offset
					hunk_id++;
					output_hunk_offset = &output_buffer[get_hunk_section_offset(hunkh, hunk_id)];
				break;

				default:
					// Error : Unknow HUNK Type...
					FreeMem(output_buffer, totalhunk_size<<2);
					return 0;
				break;
			}
		}

		// Check that all Hunk have been processed.
		if (hunk_id != hunkh->NB_OF_HUNKS)
		{
			FreeMem(output_buffer, totalhunk_size<<2);
			return 0;
		}

		return output_buffer;
	}

	return 0;
}

int bootblock_main(params * paramszone)
{
	unsigned char * loading_mem;
	int block, out_offset;
	uint32_t checksum;
	int retry;
	struct IOStdReq *ioreq;
	uint32_t * entry_point,loopcnt;

	retry = 3;
	ioreq = (struct IOStdReq *)paramszone->ioreq;

	// Blue background...
	*((volatile unsigned short *)COLOR00_ADDR) = BG_COLOR_PROGRESS;

	loading_mem = AllocMem(paramszone->total_blocks_size, MEMF_CHIP|MEMF_CLEAR|MEMF_PUBLIC);
	if( loading_mem )
	{
		while(retry)
		{
			// Load all group of sectors indicated into the block table.
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
					*((volatile unsigned short *)COLOR00_ADDR) = BG_COLOR_ERROR_1;
					retry--;
					break;
				}

				out_offset += paramszone->blocktable[block].lenght;

				block++;
			}

			// Remove the sectors headers...
			pack_payload((uint32_t *) loading_mem, ((paramszone->exec_size)>>2)+1);

			// Compute the payload checksum
			checksum = payload_checksum ((uint32_t *) loading_mem, (paramszone->exec_size)>>2 );

			if( paramszone->exec_checksum == checksum )
			{
				// Good ! -> Parsing the Hunk
				entry_point = load_hunk((uint32_t * )loading_mem,(paramszone->exec_size>>2));

				// Free the loaded file.
				FreeMem(loading_mem, paramszone->total_blocks_size);

				if(entry_point)
				{
					*((volatile unsigned short *)COLOR00_ADDR) = BG_COLOR_OK;

					// and start the program !
					Jump_To_Exec(entry_point, (struct IOStdReq *)paramszone->ioreq);
					// TODO -> Manage case of program exit.
					for(;;);
				}
				else
				{
					*((volatile unsigned short *)COLOR00_ADDR) = BG_COLOR_ERROR_3;
					return 1; // Hunk loading failure...
				}
			}
			else
			{
				*((volatile unsigned short *)COLOR00_ADDR) = BG_COLOR_ERROR_2;
				retry--;
			}
		}

		// 3x Error...
		FreeMem(loading_mem, paramszone->total_blocks_size);
	}

	// Something wrong happened... Red alert...
	loopcnt = 0;
	for(;;)
	{
		*((volatile unsigned short *)COLOR00_ADDR) = (loopcnt>>5) & 0xF00;
		loopcnt++;
	}
	return 1;
}
