asm(
"	.text;"
"	.even;"
"	.globl	_longjmp;"

"_longjmp:"
"	addql	#4,sp;"			/* returns to another address */
"	movel	sp@+,a0;"		/* get address of jmp_buf */
"	movel	sp@+,d2;"		/* get returncode */
"	movel	a0@(48:W),d0;"
"	jbsr	___stkrst;"		/* restore sp */
"	movel	d2,d0;"
"	jne	l0;"			    /* != 0 -> ok */
"	moveql	#1,d0;"
"l0:	movel	a0@+,sp@;"	/* set returnaddress */
"	moveml	a0@,#0x7cfc;"		/* restore all registers except scratch and sp */
"	rts"
);
