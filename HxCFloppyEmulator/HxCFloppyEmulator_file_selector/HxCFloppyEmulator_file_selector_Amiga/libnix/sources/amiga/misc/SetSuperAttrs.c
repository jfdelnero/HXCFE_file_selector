asm(
"		.text;"

"		.globl	_SetSuperAttrs;"

"_SetSuperAttrs:	movel	a2,sp@-;"
"		moveml	sp@(8:W),a0/a2;"
"		movel	a2,d0;"
"		beqs	L_Null;"
"		movel	a0,d0;"
"		beqs	L_Null;"
"		clrl	sp@-;"
"		pea	sp@(20:W);"
"		pea	259:W;"
"		movel	sp,a1;"
"		movel	sp@(24:W),a0;"
"		jbsr	L_Invoke;"
"		lea	sp@(12:W),sp;"
"L_Null:		movel	sp@+,a2;"
"		rts;"

"L_Invoke:	movel	a0@(8:W),sp@-;"
"		rts;"
);
