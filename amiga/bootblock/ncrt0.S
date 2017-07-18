| Minimal ncrt0.S for Amiga bootblock env
| (c)2017 JF.DEL NERO / HxC2001

	.globl	_SysBase
	.globl	_ioreq

	.text

start:                          | Entry point
    lea     _SysBase(pc), a0    | Save SysBase + Ioreq for the C usage
	move.l  a6,  (a0)
	move.l  a1, 4(a0)

	bsr.w   _bootblock_main     | Enter C "int bootblock_main(void)" function.
	                            | D0 should contain the bootblock_main return code...
	                            | If D0 != 0 -> Error !
	lea     start(pc),a0        | Restart point...
	rts

_SysBase:   dc.l    0
_ioreq:     dc.l    0
