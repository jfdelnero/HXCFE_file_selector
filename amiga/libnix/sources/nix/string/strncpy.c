#ifndef mc68000

#include <string.h>

char *strncpy(char *s1,const char *s2,size_t n)
{
  if (n) {
    char *s=s1;
    do;while((*s++=*s2++) && --n);
    if (n)
      while(--n) *s++=0;
  }
  return s1;
}

#else

asm(
"	.globl	_strncpy;"
"_strncpy:;"
"	moveml	sp@(4:W),d0/a0;"
"	movel	d0,a1;"
"	movel	sp@(12:W),d1;"
"L4:	subql	#1,d1;"
"	bcs	L1;"
"	moveb	a0@+,a1@+;"
"	bne	L4;"
"	.word	0x0c40;"
"L3:	clrb	a1@+;"
"	subql	#1,d1;"
"	bcc	L3;"
"L1:	rts;"
);

#endif
