asm(
"		.text;"

"		.globl	_CallHook;"
"		.globl	_CallHookA;"

"_CallHook:	lea	sp@(12:W),a1;"
"		jra	L_CallHook;"

"_CallHookA:	movel	sp@(12:W),a1;"

"L_CallHook:	movel	a6,sp@-;"
"		movel	a2,sp@-;"
"		movel	sp@(12:W),a0;"
"		movel	sp@(16:W),a2;"
"		jbsr	L_callit;"
"		movel	sp@+,a2;"
"		movel	sp@+,a6;"
"		rts;"

"L_callit:	movel	a0@(8:W),sp@-;"
"		rts;"
);
