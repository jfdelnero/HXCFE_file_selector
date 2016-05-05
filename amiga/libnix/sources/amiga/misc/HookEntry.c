#if 0

#include <utility/hooks.h>

ULONG HookEntry(struct Hook *hook asm("a0"),APTR obj asm("a2"),APTR msg asm("a1"))
{
  return (*hook->h_SubEntry)(hook,obj,msg);
}

#else

asm(
"		.text;"

"		.globl	_HookEntry;"

"_HookEntry:	movel	a1,sp@-;"
"		movel	a2,sp@-;"
"		movel	a0,sp@-;"
"		movel	a0@(12:W),a0;"
"		jsr	a0@;"
"		lea	sp@(12:W),sp;"
"		rts;"
);

#endif
