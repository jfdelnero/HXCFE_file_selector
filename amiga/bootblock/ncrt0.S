| Minimal ncrt0.S for Amiga bootblock env
| (c)2018 JF.DEL NERO / HxC2001

	.globl _SysBase

	.text

start:                                   | Entry point

	lea     _GlobalParamsArea(pc), a0    | Save SysBase + Ioreq for the C usage
	move.l  a6,  (a0)                    | SysBase
	move.l  a1, 4(a0)                    | IOReq

	move.l  a0,-(sp)                     | Push the Params Area offset to the function parameter
	lea     _bootblock_main(pc),a0       | Get/compute the main function absolute offset.
	jsr     (a0)                         | Enter the C "int bootblock_main(params * paramszone)" function.
	addq.l  #4,sp                        | Fix the Stack pointer.
	                                     | D0 should contain the bootblock_main return code...
	                                     | If D0 != 0 -> Error !
	lea     start(pc),a0                 | Restart point...
	rts                                  | Return to the Kickstart

.align 4                                 | All datas after this limit be dword aligned !

.ascii       "<HXCFEM>"                  | Param Area marker (8 Bytes)

_GlobalParamsArea:
_SysBase:    dc.l    0
_ioreq:      dc.l    0
_Params:     ds.l    20                  | (4 Entries * 2 longs) + 3*4 = 20 longs
