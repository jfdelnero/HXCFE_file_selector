asm(
"		.text;"

"		.globl	_CoerceMethod;"
"		.globl	_CoerceMethodA;"

"_CoerceMethod:	lea	sp@(12:W),a1;"
"		jra	L_CoerceMethod;"

"_CoerceMethodA:	movel	sp@(12:W),a1;"

"L_CoerceMethod:	movel	a2,sp@-;"
"		moveml	sp@(8:W),a0/a2;"
"		movel	a0,d0;"
"		beqs	L_Null;"
"		movel	a2,d0;"
"		beqs	L_Null;"
"		jbsr	L_Invoke;"
"L_Null:		movel	sp@+,a2;"
"		rts;"

"L_Invoke:	movel	a0@(8:W),sp@-;"
"		rts;"
);
