#include "bases.h"

asm(
	".data;"
	".even;"
"body:	.byte	0,0,0,0;"
"	.word	15,5;"
"	.long	0,0,0;"

"ok:	.byte	0,0,0,0;"
"	.word	6,3;"
"	.long	0,oktext,0;"
"	.text;"
"oktext:	.byte	79,107,0;"

"	.even;"
"	.globl	___request;"
"___request:"
"	moveml	#0x3832,sp@-;"
"	movel	4:W,a6;"
"	subal	a1,a1;"
"	jsr	a6@(-0x126);"			/* FindTask(a1:nam) */
"	movel	d0,a1;"
"	moveq	#-1,d1;"
"	cmpl	a1@(0xb8:W),d1;"
"	jeq	l_fail;"				/* Dont put requesters up */
"	lea	"A4(___intuitionname)",a1;"
"	moveq	#0,d0;"
"	jsr	a6@(-0x228);"			/* OpenLibrary(a1:nam,d0:ver) */
"	movel	d0,d4;"
"	jeq	l_fail;"
"	lea	"A4(ok)",a3;"
"	lea	"A4(body)",a1;"
"	subl	a2,a2;"
"	movel	sp@(28),a1@(12:W);"
"	subl	a0,a0;"
"	moveq	#0,d0;"
"	moveq	#0,d1;"
"	moveq	#(640>>4),d2;"
"	lslw	#4,d2;"
"	moveq	#72,d3;"
"	exg	d4,a6;"
"	jsr	a6@(-0x15c);"			/* AutoRequest(a0,a1,a2,a3,d0,d1,d2,d3) */
"	movel	a6,a1;"
"	movel	d4,a6;"
"	jsr	a6@(-0x19e);"			/* CloseLibrary(a1:lib) */
"l_fail:	moveml	sp@+,#0x4c1c;"
"	rts;"
);
