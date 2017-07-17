| Minimal ncrt0.S for Amiga bootblock env
| (c)2017 JF.DEL NERO / HxC2001


	.text

start:                     | Entry point
	movel   a6,_SysBase    | Save SysBase + Ioreq for the C usage
	movel   a1,_ioreq

	jbsr    _bootblock_main| Enter C void bootblock_main(void).
	                       | D0 should contain the main return code...
	                       | If D0 != 0 -> Error !
	lea     start(pc),a0   | Restart point...
	rts

