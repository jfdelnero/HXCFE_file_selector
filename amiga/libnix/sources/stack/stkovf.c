#include "bases.h"

asm(
"	.text;"
"	.even;"
"	.globl	___stkovf;"

"___stkovf:;"
"	clrl	"A4(___stk_limit)";"	/* generate no more stackoverflows */
"	jra	__XCOVF;"
);
