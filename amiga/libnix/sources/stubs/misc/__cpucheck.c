#include "bases.h"
#include "stabs.h"

#if defined(mc68020) || defined(__HAVE_68881__)

#ifdef mc68020
const char errtext1[]="Need a 68020 or higher CPU";
#endif

#ifdef __HAVE_68881__
const char errtext2[]="Need some FPU to run";
#endif

asm(
"	.text;"
"	.even;"

"	.globl	___cpucheck;"
"___cpucheck:;"
"	movel	"A4(_SysBase)",a1;"
"	movew	a1@(0x128:W),d0;"

#ifdef mc68020

"	lea	_errtext1,a0;"
"	btst	#1,d0;"
"	jeq	error;"

#endif
#ifdef __HAVE_68881__

"	lea	_errtext2,a0;"
"	btst	#4,d0;"
"	jeq	error;"

#endif

"	rts;"
"error:"
"	movel	a0,sp@-;"
"	jsr	___request;" /* jbsr translates to jra which doesn't work on 68000 */
"	pea	20:W;"
"	jsr	_exit;"		/* dito */
);

//ADD2INIT(__cpucheck,-80); /* Highest priority */

#else

int __cpucheck;

#endif
