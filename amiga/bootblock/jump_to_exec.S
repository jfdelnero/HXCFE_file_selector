/* ---------------- */
/*  (c)JF.DEL NERO  */
/*  (c)HxC2001      */
/* ---------------- */

| public symbols

.globl	_Jump_To_Exec

.text
// void Jump_To_Exec(void * entry,void * ioreq)

_Jump_To_Exec:
	arg_0=  4
	arg_4=  8

	move.l  a6,-(sp)

	movea.l _SysBase(pc),a6
	move.l  4+arg_0(sp),a0       | entry point
	move.l  4+arg_4(sp),a1       | ioreq

	jsr     (a0)

	movea.l (sp)+,a6
	rts
