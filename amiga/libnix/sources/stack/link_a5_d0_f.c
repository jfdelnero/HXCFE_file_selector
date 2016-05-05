#include "bases.h"

asm(
"	.text;"
"	.even;"
"	.globl	___link_a5_d0_f;"
"	.globl	___sub_d0_sp_f;"

"___link_a5_d0_f:"
"	movel	sp@+,a0;"
"	movel	sp,d1;"
"	subl	d0,d1;"
"	cmpl	"A4(___stk_limit)",d1;"
"	jcc	l0;"
"	jbsr	l2;"
"l0:	link	a5,#0:W;"
"	subl	d0,sp;"
"	jmp	a0@;"

"___sub_d0_sp_f:"
"	movel	sp@+,a0;"
"	movel	sp,d1;"
"	subl	d0,d1;"
"	cmpl	"A4(___stk_limit)",d1;"
"	jcc	l1;"
"	jbsr	l2;"
"l1:	subl	d0,sp;"
"	jmp	a0@;"

"l2:	moveq	#0,d1;"
"	jra	___stkext_f;"
);
