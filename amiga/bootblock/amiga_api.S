/* ---------------- */
/*  Amiga API calls */
/*  (c)JF.DEL NERO  */
/*  (c)HxC2001      */
/* ---------------- */

_LVOAllocMem = -198
_LVOFreeMem = -210

| public symbols

.globl	_AllocMem
.globl	_FreeMem
.globl	_OpenLibrary
.globl	_DoIO

.text

/* Called at the main() entry function */

/* ---------- SysBase ------------ */

_AllocMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase(pc),a6
	move.l  4+arg_0(sp),d0
	move.l  4+arg_4(sp),d1
	jsr		a6@(_LVOAllocMem:W)
	movea.l (sp)+,a6
	rts

_FreeMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase(pc),a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr		a6@(_LVOFreeMem:W)
	movea.l (sp)+,a6
	rts

_OpenLibrary:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _SysBase(pc),a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr     -0x228(a6)
	movea.l (sp)+,a6
	rts

_DoIO:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _SysBase(pc),a6
	move.l  4+arg_0(sp),a1
	jsr     -0x1c8(a6)
	movea.l (sp)+,a6
	rts

/* ---------- Dos.Library ------------ */
/*
_PutStr:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _DOSBase(pc),a6
	move.l  4+arg_0(sp),d1
	jsr     -0x3B4(a6)
	movea.l (sp)+,a6
	rts
*/
