#ifndef mc68000

#include <string.h>

char *strcpy(char *s1,const char *s2)
{ char *s=s1;
#if 0
  do;while(*s1++=*s2,*s2++!='\0');
#else
  do;while((*s1++=*s2++));
#endif
  return s;
}

#else

asm(
"	.globl	_strcpy;"
"_strcpy:;"
"	moveml	sp@(4:W),d0/a0;"
"	movel	d0,a1;"
"L1:	moveb	a0@+,a1@+;"
"	jne	L1;"
"	rts;"
);

#endif
