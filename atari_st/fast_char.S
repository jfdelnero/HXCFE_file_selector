|
| Copyright (C) 2009-2021 Jean-François DEL NERO
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

|
| Fast character functions - Atari ST version. Low/med & High resolution support.
|

| ----------------------------------------------------------------------------------

	.globl _fast_print_char
	.align 4
	.text

_fast_print_char:

	move.l  4(sp), a1 | First parameter -> Destination.
	move.l  8(sp), a0 | Second parameter -> Source.

	| ! The above instructions are modified by the C code !
	| See the patch_char_func function.

	move.l  #0xAAAA, d0

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	move.b  (a0)+,(a1)
	add.l   d0,a1

	rts

| ----------------------------------------------------------------------------------

	.globl _fast_print_inv_char
	.align 4
	.text

_fast_print_inv_char:

	move.l    4(sp), a1 | First parameter -> Destination.
	move.l    8(sp), a0 | Second parameter -> Source.

	| ! The above instructions are modified by the C code !
	| See the patch_char_func function.

	move.l  #0xAAAA, d0

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	move.b    (a0)+,d1
	not.b     d1
	move.b    d1,(a1)
	add.l     d0,a1

	rts

| ----------------------------------------------------------------------------------

	.globl _fast_clear_line
	.align 4
	.text

_fast_clear_line:

	move.l    4(sp), a0      | First parameter  -> Destination.
	move.l    8(sp), d1      | Second parameter -> Fill value.
	move.l    12(sp), d0     | Third parameter  -> line per char.

	movem.l   d2,-(sp)

	move.l    (16+4)(sp), d2 | fourth parameter -> nb bitplanes
	lsl.l     #1,d2          | words to bytes


	lsr.l     #3,d0    | 8 words per loop
	subq.l    #1,d0

clearloop_l1:
    move.w    d1,(a0)
	adda.l    d2,a0

    move.w    d1,(a0)
	adda.l    d2,a0

	move.w    d1,(a0)

	adda.l    d2,a0

	move.w    d1,(a0)
	adda.l    d2,a0

	move.w    d1,(a0)
	adda.l    d2,a0

	move.w    d1,(a0)
	adda.l    d2,a0

	move.w    d1,(a0)
	adda.l    d2,a0

	move.w    d1,(a0)
	adda.l    d2,a0

	| more lines to clear ?
	dbra      d0, clearloop_l1

	movem.l   (sp)+,d2

	rts

| ----------------------------------------------------------------------------------

	.globl _fast_inverse_line
	.align 4
	.text

_fast_inverse_line:

	move.l    4(sp),  a0 | First parameter -> Destination.
	move.l    8(sp),  d0 | Second parameter -> line per char.
	move.l    12(sp), d1 | third parameter -> nb bitplans

	lsl.l     #1,d1     | words to bytes

	lsr.l     #3,d0    | 8 words per loop
	subq.l    #1,d0

fast_inverse_line_l1:

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	not.w     (a0)
	adda.l    d1,a0

	| more words to inverse ?
	dbra      d0, fast_inverse_line_l1

	rts
