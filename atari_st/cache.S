|
| This file is part of the HxCFloppyEmulator file selector.
|
| HxCFloppyEmulator file selector may be used and distributed without restriction
| provided that this copyright statement is not removed from the file and that any
| derivative work contains the original copyright notice and the associated
| disclaimer.
|
| HxCFloppyEmulator file selector is free software; you can redistribute it
| and/or modify  it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| HxCFloppyEmulator file selector is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
|   See the GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with HxCFloppyEmulator file selector; if not, write to the Free Software
| Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|

	.globl _is_68020
	.align 4
	.text

	| From https://groups.google.com/d/topic/comp.sys.m68k/SF0ektrtBtQ/discussion (Detecting 68040/060)
	_is_68020:
		movem.l a7,-(a7)    | Don't let this line get 'optimised'
		cmpa.l  (a7)+,a7	| Was stored value pre-decremented?
		bne.s   MC020		| Z unless this is a 68020 or later

		move.l #0,d0		| Not an 68020+

		rts

	MC020:
		move.l #1,d0		| This is a 68020+

		rts

| --------------------------------------------------------------------------------------------------------

	.globl _invalidate_icache
	.align 4
	.text

	| amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node000C.html

	| Must be executed in supervisor mode

	_invalidate_icache:

		dc.w    0x4E7A,0x0002     |MOVEC CACR,D0
		tst.w   d0                |movec does not affect CC's
		bmi.s   cic_040           |A 68040 with enabled cache!

		ori.w   #0x0008,d0        |Bit for clear instruction cache
		dc.w    0x4E7B,0x0002     |MOVEC D0,CACR
		bra.s   cic_exit

	cic_040:
		dc.w    0xf4b8            |CPUSHA (IC)

	cic_exit:
		nop

		rts
