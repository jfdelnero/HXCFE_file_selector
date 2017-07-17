/* ---------------- */
/*  Amiga API calls */
/*  (c)JF.DEL NERO  */
/*  (c)HxC2001      */
/* ---------------- */

_LVOForbid = -132
_LVOPermit = -138
_LVOAllocMem = -198
_LVOFreeMem = -210
_LVOGetProgramDir = -600

| public symbols

.globl	_Forbid
.globl	_Permit
.globl	_AllocMem
.globl	_FreeMem
.globl	_AddIntServer
.globl	_OpenLibrary

.globl	_Info
.globl	_Lock
.globl	_UnLock
.globl	_GetProgramDir
.globl	_Delay

.globl	_FreeCprList
.globl	_FreeVPortCopLists
.globl	_FreeColorMap
.globl	_FreeRaster
.globl	_LoadRGB4
.globl	_InitBitMap
.globl	_InitView
.globl	_InitVPort
.globl	_AllocRaster
.globl	_BltClear
.globl	_GetColorMap
.globl	_MakeVPort
.globl	_MrgCop
.globl	_LoadView
.globl	_WaitTOF
.globl	_InitRastPort
.globl	_SetAPen

.globl	_OpenScreen

.text

/* Called at the main() entry function */

/* ---------- SysBase ------------ */

_Forbid:
	move.l  a6,-(sp)
	movea.l _SysBase,a6
	jsr	a6@(_LVOForbid:W)
	movea.l (sp)+,a6
	rts

_Permit:
	move.l  a6,-(sp)
	movea.l _SysBase,a6
	jsr	a6@(_LVOPermit:W)
	movea.l (sp)+,a6
	rts

_AllocMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),d0
	move.l  4+arg_4(sp),d1
	jsr		a6@(_LVOAllocMem:W)
	movea.l (sp)+,a6
	rts

_FreeMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr		a6@(_LVOFreeMem:W)
	movea.l (sp)+,a6
	rts

_AddIntServer:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),d0
	movea.l 4+arg_4(sp),a1
	jsr     -0xA8(a6)
	movea.l (sp)+,a6
	rts

_OpenLibrary:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr     -0x228(a6)
	movea.l (sp)+,a6
	rts

/* ---------- Dos.Library ------------ */
_Info:

	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	move.l  d2,-(sp)
	movea.l _DOSBase,a6
	move.l  8+arg_0(sp),d1
	move.l  8+arg_4(sp),d2
	jsr		a6@(_LVOGetProgramDir:W)
	move.l  (sp)+,d2
	movea.l (sp)+,a6
	rts

_Lock:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	move.l  d2,-(sp)
	movea.l _DOSBase,a6
	move.l  8+arg_0(sp),d1
	move.l  8+arg_4(sp),d2
	jsr     -0x54(a6)
	move.l  (sp)+,d2
	movea.l (sp)+,a6
	rts

_UnLock:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _DOSBase,a6
	move.l  4+arg_0(sp),d1
	jsr     -0x5A(a6)
	movea.l (sp)+,a6
	rts

_GetProgramDir:
	move.l  a6,-(sp)
	movea.l _DOSBase,a6
	jsr     -0x258(a6)
	movea.l (sp)+,a6
	rts

_Delay:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _DOSBase,a6
	move.l  4+arg_0(sp),d1
	jsr     -0xC6(a6)
	movea.l (sp)+,a6
	rts

/* ---------- Graphics.library ------------ */

_FreeCprList:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	jsr     -0x234(a6)
	movea.l (sp)+,a6
	rts

_FreeVPortCopLists:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	jsr     -0x21C(a6)
	movea.l (sp)+,a6
	rts

_FreeColorMap:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	jsr     -0x240(a6)
	movea.l (sp)+,a6
	rts

_FreeRaster:
	arg_0           =  4
	arg_4           =  8
	arg_8           =  0xC

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	move.l  4+arg_4(sp),d0
	move.l  4+arg_8(sp),d1
	jsr     -0x1F2(a6)
	movea.l (sp)+,a6
	rts

_LoadRGB4:
	arg_0           =  4
	arg_4           =  8
	arg_8           =  0xC

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	movea.l 4+arg_4(sp),a1
	move.l  4+arg_8(sp),d0
	jsr     -0xC0(a6)
	movea.l (sp)+,a6
	rts

_InitView:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	jsr     -0x168(a6)
	movea.l (sp)+,a6
	rts

_InitBitMap:
	arg_0           =  4
	arg_4           =  8
	arg_8           =  0xC
	arg_C           =  0x10

	move.l  a6,-(sp)
	move.l  d2,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 8+arg_0(sp),a0
	move.l  8+arg_4(sp),d0
	move.l  8+arg_8(sp),d1
	move.l  8+arg_C(sp),d2
	jsr     -0x186(a6)
	move.l  (sp)+,d2
	movea.l (sp)+,a6
	rts

_InitVPort:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	jsr     -0xCC(a6)
	movea.l (sp)+,a6
	rts

_AllocRaster:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	move.l  4+arg_0(sp),d0
	move.l  4+arg_4(sp),d1
	jsr     -0x1EC(a6)
	movea.l (sp)+,a6
	rts


_BltClear:
	arg_0           =  4
	arg_4           =  8
	arg_8           =  0xC

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	move.l  4+arg_8(sp),d1
	jsr     -0x12C(a6)
	movea.l (sp)+,a6
	rts

_GetColorMap:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	move.l  4+arg_0(sp),d0
	jsr     -0x23A(a6)
	movea.l (sp)+,a6
	rts

_MakeVPort:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a0
	movea.l 4+arg_4(sp),a1
	jsr     -0xD8(a6)
	movea.l (sp)+,a6
	rts

_MrgCop:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	jsr     -0xD2(a6)
	movea.l (sp)+,a6
	rts

_LoadView:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	jsr     -0xDE(a6)
	movea.l (sp)+,a6
	rts

_WaitTOF:
	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	jsr     -0x10E(a6)
	movea.l (sp)+,a6
	rts

_InitRastPort:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	jsr     -0xC6(a6)
	movea.l (sp)+,a6
	rts

_SetAPen:
	arg_0           =  4
	arg_4           =  8

	move.l  a6,-(sp)
	movea.l _GfxBaseptr,a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr     -0x156(a6)
	movea.l (sp)+,a6
	rts

/* ---------- Intuition.library ------------ */

_OpenScreen:
	arg_0           =  4

	move.l  a6,-(sp)
	movea.l _IntuitionBase,a6
	movea.l 4+arg_0(sp),a0
	jsr     -0xC6(a6)
	movea.l (sp)+,a6
	rts
