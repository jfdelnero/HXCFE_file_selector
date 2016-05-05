#include <stdlib.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include "debuglib.h"

extern struct MinList __memorylist;
extern struct SignalSemaphore *__memsema;

void free(void *ptr)
{ struct MemHeader *a;

  if(!ptr) /* What does that mean ????? */
  { DB( BUG("NULL pointer free'd\n"); )
    return; }

  ObtainSemaphore(__memsema);
  a=(struct MemHeader *)__memorylist.mlh_Head;
  for(;;)
  {
    if(((struct MinNode *)a)->mln_Succ==NULL) /* Is not in list ????? */
    { DB( BUG("Fake memory free'd\n"); )
      return; }

    if(ptr>=a->mh_Lower&&ptr<a->mh_Upper) /* Entry found */
      break;

    a=(struct MemHeader *)((struct MinNode *)a)->mln_Succ;
  }

  DB( memset(ptr,0xcc,((ULONG *)ptr)[-1]); ) /* Destroy contents */

  Deallocate(a,(ULONG *)ptr-1,((ULONG *)ptr)[-1]);
  if(a->mh_Free==(char *)a->mh_Upper-(char *)a->mh_Lower) /* All free ? */
  { Remove(&a->mh_Node);
    FreeMem(a,(char *)a->mh_Upper-(char *)a); 
  }
  ReleaseSemaphore(__memsema);
}
