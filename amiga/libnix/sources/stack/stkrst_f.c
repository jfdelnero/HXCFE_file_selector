#include "bases.h"

asm(
"	.text;"
"	.even;"
"	.globl ___stkrst_f;"

"___stkrst_f:;"
"	movel	a2,sp@-;"
"	moveml	#0xc0c2,sp@-;"		/* Preserve all registers; */
"	movel	sp,a2;"
"	lea	"A4(___used_stack)",a1;" /* Move current stackframe to the spares list */
"	movel	a1@,a0;"
"	movel	a0@,a1@+;"
"	movel	a1@,a0@;"
"	movel	a0,a1@;"
"	addql	#4,a0;"			/* Return to old stackframe */
"	movel	a0@(12:W),"A4(___stk_limit)";"
"	movel	"A4(_SysBase)",a6;"
"	jsr	a6@(-0x2dc);"		/* StackSwap(sss:a0) */
"	moveml	a2@+,#0x4303;"
"	movel	a2@,a2;"
"	rts;"
);
