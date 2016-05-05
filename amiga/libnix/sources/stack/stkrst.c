	#include "bases.h"

asm(
	"| the stack consists of a single linked linear list like this:"
	"|"
	"| struct stack"
	"| { struct stack *next;			points to the next underlying structure"
	"|   struct StackSwapStruct sss;		holds previous stackframe when used,"
	"|					itself when unused, the current stackborders"
	"|					are in the task structure ;-)"
	"|   APTR top_of_stackframe; }		previous variable value"
	"|					Note: stack->sss.stk_Lower is not reliable"
	"|"
	"| For better performance stackframes are cached in a 'unused' list when"
	"| not in use. (AllocMem() is really a performance killer)."
	"|"
	"|"
	"| Caution:"
	"| Race condition ahead! exec might preempt our task at any time storing"
	"| the current register set on top of the free stack. NEVER set a sp higher"
	"| than the location of important data."
	"|"
	"	.comm	___used_stack,8;"		/* pointer to used stackframes */
	                                    /* pointer to unused stackframes */
	"	.text;"
	"	.even;"
	"	.globl	___stkrst;"

	"__stkrst:;"
	"	exg	d0,a3;"			/* better use an address register*/
	"	movel	a2,sp@-;"
	"	moveml	d0/d1/a0/a1/a5/a6,sp@-;"
	"	movel	sp,a2;"
	"	lea	"A4(___used_stack)",a5;"
	"	tstl	a5@;"
	"	jeq	l0;"			/* No previous stackframe*/
	"l2:	movel	"A4(___stackborders)",a0;"
	"	cmpl	a0@,a3;"
	"	jcs	l1;"
	"	cmpl	a0@(4:W),a3;"
	"	jle	l0;"			/* Stackpointer points to current frame*/
	"l1:	movel	"A4(_SysBase)",a6;"	/* Go to previous stack*/
	"	movel	a5,a1;"
	"	movel	a1@,a0;"
	"	movel	a0@,a1@+;"
	"	movel	a1@,a0@;"
	"	movel	a0,a1@;"
	"	addql	#4,a0;"
	"	movel	a0@(12:W),"A4(___stk_limit)";"
	"	jsr	a6@(-0x2dc);"		/* StackSwap(sss:a0)*/
	"	tstl	a5@;"
	"	jne	l2;"
	"l0:	moveml	a2@+,d0/d1/a0/a1/a5/a6;"	/* Restore registers*/
	"	movel	a2@(4:W),sp@-;"		/* preserve returnaddress on current stackframe*/
	"	movel	a2@,a2;"			/* be careful to not clobber any registers now*/
	"	cmpl	sp,a3;"			/* Depending on whether sp moves up or down*/
	"	jls	l3;"			/* use one of two possible routines*/
	"	movel	sp@,a3@-;"		/* moves up (pop): copy returnaddress first*/
	"	movel	a3,sp;"			/*                 then set sp*/
	"	jra	l4;"
	"l3:	exg	a3,sp;"			/* moves down (push): set sp first*/
	"	movel	a3@,sp@-;"		/*                    then copy returnaddress*/
	"	movel	sp,a3;" 
   "l4:	exg	d0,a3;"			/* move back*/
	"	addql	#4,d0;"			/* compensate for returnaddress*/
	"	rts;"
);
