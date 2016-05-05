#include <proto/exec.h>
#include <proto/dos.h>
#include "stabs.h"

extern ULONG __stk_safezone;

APTR *__stackborders=0,__stk_limit=0;

/*
 * This way to find the bottom of the current stackframe
 * is the way descibed in the amiga-guru-book.
 *
 * Note that it's of no use to check if the actual parameters are set
 * correctly (disabling stackextension if not) because a program will
 * crash anyway in that case (if you check against the wrong border or
 * not at all).
 * But with wrong parameters you can probably raise a stackextend too
 * soon - thus gaining *correct* (for following stackextends) stackbounds
 * at the cost of only some memory.
 */

void __init_stk(void)
{
  struct Process *me = (struct Process *)FindTask(NULL);
  UBYTE *sl;

  __stackborders = (APTR *)(sl = (UBYTE *)&me->pr_Task.tc_SPLower);
  if (me->pr_CLI)
    sl = /*sizeof(ULONG)+*/ (UBYTE *)me->pr_ReturnAddr - *(ULONG *)me->pr_ReturnAddr;
         /*^^^^^^^^^^^^^^ need not be that precise */
  __stk_limit = (sl += __stk_safezone);
}

ADD2INIT(__init_stk,-50);
