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
_LVOCreateMsgPort = -666
_LVODeleteMsgPort = -672
_LVOCreateIORequest = -654
_LVODeleteIORequest = -660
_LVOOpenDevice = -444
_LVOCloseDevice = -450
_LVOFindName = -276
_LVORemDevice = -438
_LVORemResource = -492
_LVOOpenResource = -498
_LVOAllocSignal = -330 
_LVOFreeSignal = -336
_LVOFindTask = -294 
_LVOAddPort = -354 


_LVOAllocUnit = -6
_LVOFreeUnit = -12
_LVOGetUnit = -18
_LVOGiveUnit = -24
_LVOGetUnitID = -30
_LVOReadUnitID = -36

| public symbols

.globl    _Forbid
.globl    _Permit
.globl    _AllocMem
.globl    _FreeMem
.globl    _AddIntServer
.globl    _OpenLibrary
.globl    _CreateMsgPort
.globl    _CreateIORequest
.globl    _OpenDevice
.globl    _CloseDevice
.globl    _DeleteIORequest
.globl    _DeleteMsgPort
.globl    _FindName
.globl    _RemDevice
.globl    _RemResource
.globl    _OpenResource
.globl    _AllocSignal
.globl    _FreeSignal
.globl    _FindTask 
.globl    _AddPort 

.globl    _Info
.globl    _Lock
.globl    _UnLock
.globl    _GetProgramDir
.globl    _Delay

.globl    _FreeCprList
.globl    _FreeVPortCopLists
.globl    _FreeColorMap
.globl    _FreeRaster
.globl    _LoadRGB4
.globl    _InitBitMap
.globl    _InitView
.globl    _InitVPort
.globl    _AllocRaster
.globl    _BltClear
.globl    _GetColorMap
.globl    _MakeVPort
.globl    _MrgCop
.globl    _LoadView
.globl    _WaitTOF
.globl    _InitRastPort
.globl    _SetAPen

.globl    _OpenScreen

.globl    ___main

.text

/* Called at the main() entry function */
___main:
rts

/* ---------- SysBase ------------ */

_CreateMsgPort:
	move.l  a6,-(sp)
	movea.l _SysBase,a6
	jsr     a6@(_LVOCreateMsgPort:W)
	movea.l (sp)+,a6
	rts

_CreateIORequest:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a0
	move.l  4+arg_4(sp),d0
	jsr     a6@(_LVOCreateIORequest:W)
	movea.l (sp)+,a6
	rts

_OpenDevice:
	arg_0=  4
	arg_4=  8
	arg_8=  12
	arg_c=  16

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a0
	move.l  4+arg_4(sp),d0
	move.l  4+arg_8(sp),a1
	move.l  4+arg_c(sp),d1
	jsr     a6@(_LVOOpenDevice:W)
	movea.l (sp)+,a6
	rts

_CloseDevice:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVOCloseDevice:W)
	movea.l (sp)+,a6
	rts

_DeleteIORequest:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a0
	jsr     a6@(_LVODeleteIORequest:W)
	movea.l (sp)+,a6
	rts

_DeleteMsgPort:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a0
	jsr     a6@(_LVODeleteMsgPort:W)
	movea.l (sp)+,a6
	rts

_FindName:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a0
	move.l  4+arg_4(sp),a1
	jsr     a6@(_LVOFindName:W)
	movea.l (sp)+,a6
	rts

_RemDevice:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVORemDevice:W)
	movea.l (sp)+,a6
	rts
	
_OpenResource:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVOOpenResource:W)
	movea.l (sp)+,a6
	rts

_RemResource:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVORemResource:W)
	movea.l (sp)+,a6
	rts

_AllocSignal:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),d0
	jsr     a6@(_LVOAllocSignal:W)
	movea.l (sp)+,a6
	rts

_FreeSignal:
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),d0
	jsr     a6@(_LVOFreeSignal:W)
	movea.l (sp)+,a6
	rts

_FindTask: 
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVOFindTask:W)
	movea.l (sp)+,a6
	rts

_AddPort: 	
	arg_0=  4

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),a1
	jsr     a6@(_LVOFindTask:W)
	movea.l (sp)+,a6
	rts
	
_Forbid:
	move.l  a6,-(sp)
	movea.l _SysBase,a6
	jsr     a6@(_LVOForbid:W)
	movea.l (sp)+,a6
	rts

_Permit:
	move.l  a6,-(sp)
	movea.l _SysBase,a6
	jsr     a6@(_LVOPermit:W)
	movea.l (sp)+,a6
	rts
	

_AllocMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	move.l  4+arg_0(sp),d0
	move.l  4+arg_4(sp),d1
	jsr     a6@(_LVOAllocMem:W)
	movea.l (sp)+,a6
	rts

_FreeMem:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)
	movea.l _SysBase,a6
	movea.l 4+arg_0(sp),a1
	move.l  4+arg_4(sp),d0
	jsr     a6@(_LVOFreeMem:W)
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
	jsr     a6@(_LVOGetProgramDir:W)
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
	cmpa.l  #0,a6
	jeq nodoslib
	jsr     -0xC6(a6)
nodoslib:
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
