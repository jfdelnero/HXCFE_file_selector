#include "bases.h"

asm(
"	.text;"
"	.even;"
"	.globl	___link_a5_0_f;"
"	.globl	___sub_0_sp_f;"

"___link_a5_0_f:"
"	movel	sp@+,a0;"
"	cmpl	"A4(___stk_limit)",sp;"
"	jcc	l0;"
"	jbsr	l2;"
"l0:	link	a5,#0:W;"
"	jmp	a0@;"

"___sub_0_sp_f:"
"	movel	sp@+,a0;"
"	cmpl	"A4(___stk_limit)",sp;"
"	jcc	l1;"
"	jbsr	l2;"
"l1:	jmp	a0@;"

"l2:	moveq	#0,d0;"
"	moveq	#0,d1;"
"	jra	___stkext_f;"
);
