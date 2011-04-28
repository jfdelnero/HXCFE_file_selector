#ifndef mc68000

#include <string.h>

char *strncat(char *s1,const char *s2,size_t n)
{
  if (n) {
    char *s=s1;
    do;while(*s++); --s;
    for(;;) {
      if (!(*s++=*s2++))
        break;
      if (!--n) {
        *s=0; break;
      }
    }
  }
  return s1;
} 

#else

asm(
"	.globl	_strncat;"
"_strncat:;"
"	moveml	sp@(4:W),d0/a0;"
"	movel	d0,a1;"
"	movel	sp@(12),d1;"
"	jeq	L1;"
"L3:	tstb	a1@+;"
"	jne	L3;"
"	subql	#1,a1;"
"L2:	moveb	a0@+,a1@+;"
"	jeq	L1;"
"	subql	#1,d1;"
"	jne	L2;"
"	clrb	a1@;"
"L1:	rts;"
);

#endif
