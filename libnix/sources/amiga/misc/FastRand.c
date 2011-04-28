#if 1

#include <exec/types.h>

ULONG FastRand(ULONG seed)
{ ULONG a=seed<<1;
  if((LONG)seed<=0)
    a^=0x1d872b41;
  return a;
}

#else

asm("
		.text

		.globl	_FastRand

_FastRand:	movel	sp@(4:W),d0
		addl	d0,d0
		bhis	L1
		eoril	#0x1D872B41,d0
L1:		rts
");

#endif
