#include "bases.h"
#include "stabs.h"

asm(
"	.text;"
"	.even;"
"	.globl	___stkext_f;"
"	.globl	___stkext;"
"	.globl	___exit_stk;"
"	.globl	___stk_free;"

"___stkext_f:;"
"	movel	a0,sp@-;"
"	movel	sp,a0;"			/* memorize sp*/
"	moveml	#0xc062,sp@-;"		/* preserves all registers*/
"	movel	sp,a2;"			/* memorize sp*/
"	addl	#28:W,d1;"		/* Used by this function itself*/
"	movel	"A4(___stackborders)",a1;"/* get upper border of stack in a1*/
"	movel	a1@(4:W),a1;"
"	tstl	"A4(___used_stack)";"
"	bne	l0;"
"	movel	"A4(___SaveSP)",a1;"
"l0:	jbsr	___stkext;"		/* change to a stackframe with d0 bytes*/
"	movel	"A4(___used_stack)",a6;"	/* fix stored sp*/
"	addl	d1,a6@(12:W);"
"	movel	a2,d0;"			/* calculate number of bytes to copy*/
"	addl	"A4(___stk_argbytes)",d0;"/* to new stackframe in d0*/
"	addl	d1,d0;"
"	cmpl	a1,d0;"			/* never copy over the bottom of the stack*/
"	jls	l1;"
"	movel	a1,d0;"
"l1:	subl	a0,d0;"			/* copy*/
"	subl	d0,sp;"
"	movel	sp,a1;"
"	movel	"A4(_SysBase)",a6;"
"	jsr	a6@(-0x270);"		/* CopyMem(src:a0,dst:a1,siz:d0)*/
"	movel	a2,a0;"
"	moveml	a0@+,#0x4603;"
"	movel	sp,a0;"			/* Prepare cleanup*/
"	addl	d1,a0;"
"	movel	#___stkrst_f,a0@(8:W);"
"	movel	sp@+,a0;"
"	rts;"

"___stkext:;"
"	movel	a0,sp@-;"
"	moveml	#0xf072,sp@-;"		/* preserves all registers*/
"	movel	"A4(___stk_safezone)",d3;"
"	lea	"A4(___stk_limit)",a2;"
"	lea	"A4(___used_stack)",a3;"
"	addql	#4,a3;"
"	movel	"A4(_SysBase)",a6;"
"	movel	d0,d2;"
"	addl	d3,d2;"
"	addl	"A4(___stk_argbytes)",d2;"/* required stack*/
"	movel	a3@,d0;"			/* Look for a spare stackframe*/
"	jeq	l4;"
"l3:	movel	d0,a0;"
"	movel	a0@,a3@;"			/* Remove spare stackframe from list*/
"	movel	a0@(8:W),d0;"		/* Test spare stackframe*/
"	subl	a0@(4:W),d0;"
"	cmpl	d0,d2;"
"	jls	l7;"
"	moveq	#20,d1;"			/* Not big enough*/
"	addl	d1,d0;"
"	movel	a0,a1;"
"	jsr	a6@(-0xd2);"		/* FreeMem(mem:a1,siz:d0)*/
"	movel	a3@,d0;"
"	jne	l3;"
"l4:	moveq	#20,d0;"
"	addl	d0,d2;"			/* no more stackframes - allocate a new one*/
"	movel	"A4(___stk_minframe)",d0;"
"	cmpl	d0,d2;"
"	jge	l5;"
"	movel	d0,d2;"
"l5:	movel	d2,d0;"
"	moveql	#1,d1;"			/* MEMF_PUBLIC*/
"	jsr	a6@(-0xc6);"		/* AllocMem(siz:d0,typ:d1) */
"	movel	d0,a0;"
"	movel	a0,d0;"
"	jne	l6;"
"	jbsr	___stkovf;"		/* allocation failed*/
"l6:	moveq	#20,d0;"
"	addl	a0,d0;"
"	movel	d0,a0@(4:W);"
"	addl	a0,d2;"			/* prepare new frame for use*/
"	movel	d2,a0@(8:W);"
"l7:	movel	a3@-,a0@;"		/* Add it to the used list*/
"	movel	a0,a3@;"
"	addql	#4,a0;"
"	movel	a2@,a0@(12:W);"
"	movel	a0@,a2@;"			/* Prepare ___top_of_stackframe variable*/
"	addl	d3,a2@;"
"	lea	a0@(4:W),a2;"
"	movel	a2@+,a2@;"		/* Prepare target sp*/
"	jsr	a6@(-0x2dc);"		/* StackSwap(sss:a0)*/
"	moveq	#40,d0;"
"	movel	a2@,a0;"
"	addl	d0,a2@;"			/* Fix stored sp*/
"	moveml	a0@+,#0x4e0f;"
"	movel	a0@(4:W),sp@-;"		/* prepare returnaddress*/
"	movel	a0@,a0;"
"	rts;"

"___exit_stk:;"
"	tstl	"A4(___used_stack)";"
"	jeq	___stk_free;"
"	lea	sp@(12:W),a0;"
"	movel	"A4(___SaveSP)",d0;"	/* Go back to first stackframe*/
"	jbsr	___stkrst;"
"	movel	a0@,sp@-;"		/* restore argument of exit()*/
"	movel	a0@-,sp@-;"		/* returnaddress for exit()*/
"	movel	a0@-,sp@-;"		/* returnaddress for callfuncs()*/
"	movel	a0@-,sp@-;"		/* our returnaddress */
"	jbsr	___stk_free;"
"	rts;"

"___stk_free:;"
"	movel	a2,sp@-;"
"	movel	a6,sp@-;"			/* Free all unused stackframes*/
"	movel	"A4(_SysBase)",a6;"
"	lea	"A4(___used_stack)",a2;"
"	addql	#4,a2;"
"	movel	a2@,d0;"			/* Remove spare stackframe from list*/
"	jeq	l9;"
"l8:	movel	d0,a1;"
"	movel	a1@,a2@;"
"	moveq	#20,d0;"
"	addl	a1@(8:W),d0;"		/* Calculate size*/
"	subl	a1@(4:W),d0;"
"	jsr	a6@(-0xd2);"		/* FreeMem(mem:a1,siz:d0)*/
"	movel	a2@,d0;"
"	jne	l8;"
"l9:	movel	sp@+,a6;"
"	movel	sp@+,a2;"
"	rts;"
);

ADD2EXIT(__exit_stk,-50);
