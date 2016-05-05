/* 10-Apr-94 bug fix M. Fleischer
 * 11-Apr-94 bug fix & readjustment G. Nikl
 * 14-Apr-94 readjustment M. Fleischer
 * 24-Apr-94 cleanup for malloc changed
 */

#include <stdlib.h>
#include <stdio.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/dos.h>
//#include <proto/alib.h>
#include "stabs.h"

extern ULONG _MSTEP;

struct MinList __memorylist; /* memorylist (empty): free needs also access */
struct SignalSemaphore  *__memsema;
void *malloc(size_t size)
{
  struct MinNode *node;
  struct MemHeader *b;
  ULONG size2,*a = NULL;

  ObtainSemaphore(__memsema);
  node=__memorylist.mlh_Head;
  size+=sizeof(ULONG);
  while(node->mln_Succ) /* yet some memory in my list ? */
  {
    if((a=Allocate((struct MemHeader *)node,size))!=NULL)
    { 
      *a++=size;
	  goto end;
    }
    node=node->mln_Succ;
  }
  size2=sizeof(struct MemHeader)+sizeof(ULONG)+size; /* Total memory needed */
  if(size2<=_MSTEP)
    size2=_MSTEP; /* Allocate a _MSTEP bytes large block if possible */
  size2=(size2+4095)&~4095; /* Blow up to full MMU Page */
  if((b=(struct MemHeader *)AllocMem(size2,MEMF_ANY))!=NULL)
  {
    b->mh_Lower=b->mh_First=(struct MemChunk *)(b+1);
    b->mh_First->mc_Next=NULL;
    b->mh_Free=b->mh_First->mc_Bytes=size2-sizeof(struct MemHeader);
    b->mh_Upper=(char *)b+size2;
    AddHead((struct List *)&__memorylist,&b->mh_Node);
    a=Allocate(b,size); /* It has to work this time */
	if (a != NULL) {
		*a++=size;
	}
  }

 end:
  ReleaseSemaphore(__memsema);
  return a;
}

void __initmalloc(void)
{
	struct Library *DOSBase = OpenLibrary("dos.library", 0);
	NewList((struct List *)&__memorylist);
	__memsema = AllocMem(sizeof(struct SignalSemaphore), MEMF_PUBLIC | MEMF_CLEAR);
	InitSemaphore(__memsema);
}

void __exitmalloc(void)
{ struct MemHeader *a;
  while((a=(struct MemHeader *)RemHead((struct List *)&__memorylist))!=NULL)
    FreeMem(a,(char *)a->mh_Upper-(char *)a); /* free all memory */
  FreeMem(__memsema, sizeof(struct SignalSemaphore));
}

ADD2EXIT(__exitmalloc,-50);
ADD2INIT(__initmalloc,-50);
