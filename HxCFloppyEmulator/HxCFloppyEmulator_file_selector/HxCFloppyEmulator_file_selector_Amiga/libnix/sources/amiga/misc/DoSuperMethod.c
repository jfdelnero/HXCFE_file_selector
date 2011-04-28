asm(
"		.text;"

"		.globl	_DoSuperMethod;"
"		.globl	_DoSuperMethodA;"


"_DoSuperMethod:	lea	sp@(12:W),a1;"
"		jra	L_DoSuperMethod;"

"_DoSuperMethodA:"
"		movel	sp@(12:W),a1;"

"L_DoSuperMethod:"
"		movel	a2,sp@-;"
"		moveml	sp@(8:W),a0/a2;"
"		movel	a2,d0;"
"		beqs	L_Null;"
"		movel	a0,d0;"
"		beqs	L_Null;"
"		movel	a0@(24:W),a0;"
"		jbsr	L_Invoke;"
"L_Null:		movel	sp@+,a2;"
"		rts;"

"L_Invoke:	movel	a0@(8:W),sp@-;"
"		rts;"
);
