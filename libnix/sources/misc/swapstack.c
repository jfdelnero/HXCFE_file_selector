#include <exec/memory.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdlib.h>
#include "stabs.h"

/*
 * swapstack.c
 *
 * A libnix startup module to swap to a new stack if the old one is not
 * big enough (minimum value set by the value of the __stack variable).
 *
 * WARNING!
 * Compile with -O3, or your Amiga will explode!
 *
 * Code derived from a stackswap module by Kriton Kyrimis (kyrimis@theseas.ntua.gr)
 * with some changes to work with the libnix startups (MF).
 * You use it at your own risk.
 *
 * Usage: Define some variable 'unsigned long __stack={desired size};'
 *	  somewhere in your code and link with this module.
 *	  The file stabs.h is in the headers directory of the libnix sources.
 */

extern unsigned long __stack;
void __request(const char *text);

static struct StackSwapStruct stack;
static char *newstack;

void __stkinit(long a)
{ APTR SysBase = *(APTR *)4;
  register char *sp asm("sp");
  ULONG size,needed=__stack;
  struct Process *pr;
  char *new;

  asm("":"=r"(sp)); /* touch sp to get compiler happy */

  /* Determine original stack size */

  pr=(struct Process *)FindTask(NULL);

  size = (char *)pr->pr_Task.tc_SPUpper - (char *)pr->pr_Task.tc_SPLower;

  if (pr->pr_CLI) {
    size = *(ULONG *)pr->pr_ReturnAddr;
  }

  if (needed <= size)
    return;

  /* Round size to next long word */
  needed = (needed+(sizeof(LONG)-1))&~(sizeof(LONG)-1);

  /* Allocate new stack */
  newstack = new = AllocVec(needed,MEMF_PUBLIC);
  if (!new) {
    __request("Couldn't allocate new stack!");
    exit(RETURN_FAIL);
  }

  /* Build new stack structure */
  size=(char *)&a-sp+2*sizeof(ULONG);
  stack.stk_Lower  =new;
  stack.stk_Upper  =(ULONG)(new+=needed);
  stack.stk_Pointer=(APTR)(new-=size);

  /* Copy required parts of old stack */
  CopyMem(sp,new,size);

  /* Switch to new stack */
  StackSwap(&stack);
}

void __stkexit(ULONG a)
{ APTR SysBase = *(APTR *)4;
  register char *sp asm("sp");
  ULONG size;
  char *new;

  asm("":"=r"(sp)); /* touch sp to get compiler happy */

  if(!(new=newstack))
    return;

  /* Prepare old stack */
  size=(char *)&a-sp+3*sizeof(ULONG);
  stack.stk_Pointer=(APTR)((char *)stack.stk_Pointer-size);

  /* Copy required parts of current stack */
  CopyMem(sp,stack.stk_Pointer,size);

  /* Switch back to old stack */
  StackSwap(&stack);

  /* And clean up */
  FreeVec(new);
}

/* The same priority as the detach module - you cannot use them both */
ADD2INIT(__stkinit,-70);
ADD2EXIT(__stkexit,-70);
