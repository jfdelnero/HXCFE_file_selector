asm(
"	.text;"
"	.even;"
"	.globl	_setjmp;"

"_setjmp:"
"	movel	sp@(4),a0;"		/* get address of jmp_buf */
"	movel	sp@,a0@+;"		/* store returnaddress */
"	moveml	d2-d7/a2-a6/sp,a0@;" /* store all registers except scratch */
"	moveql	#0,d0;"			/* return 0 */
"	rts;"
);
