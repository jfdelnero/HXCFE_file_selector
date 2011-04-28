#include "bases.h"

asm(
"	.text;"
"	.even;"
"	.globl	___stkchk_d0;"
"	.globl	___stkchk_0;"

"__stkchk_d0:;"
"	negl	d0;"
"	addl	sp,d0;"
"	cmpl	"A4(___stk_limit)",d0;"
"	jcs	L0;"
"	rts;"

"___stkchk_0:;"
"	cmpl	"A4(___stk_limit)",sp;"
"	jcs	L0;"
"	rts;"

"L0:	jra	___stkovf;"
);
