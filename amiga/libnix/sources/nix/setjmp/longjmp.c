asm(
"	.text;"
"	.even;"
"	.globl	_longjmp;"

"_longjmp:;"
"	addql	#4,sp;"			/* returns to another address */
"	movel	sp@+,a0;" 		/* get address of jmp_buf */
"	movel	sp@+,d0;" 		/* get returncode */
"	jne	l0;"			    /* != 0 -> ok */
"	moveql	#1,d0;"
"l0:	movel	a0@(48:W),sp;" /* restore sp */
"	movel	a0@+,sp@;"		   /* set returnaddress */
"	moveml	a0@,d2-d7/a2-a6;"  /* restore all registers except scratch and sp */
"	rts;"
);
