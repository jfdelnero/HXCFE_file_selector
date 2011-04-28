#include "pool.h"

#define NEWLIST(l) ((l)->mlh_Head = (struct MinNode *)&(l)->mlh_Tail, \
                    (l)->mlh_Tail = NULL, \
                    (l)->mlh_TailPred = (struct MinNode *)&(l)->mlh_Head)

APTR ASM AsmCreatePool(REG(d0,ULONG requirements),REG(d1,ULONG puddleSize),REG(d2,ULONG threshSize),REG(a6,APTR SysBase))
{
  if (((struct Library *)SysBase)->lib_Version>=39)
    return (CreatePool(requirements,puddleSize,threshSize));
  else {
    POOL *pool=NULL;
    if (threshSize<=puddleSize) {
      puddleSize+=7;
      if ((pool=(POOL *)AllocMem(sizeof(POOL),MEMF_ANY))!=NULL) {
        NEWLIST(&pool->PuddleList);
        pool->MemoryFlags=requirements;
        pool->PuddleSize=puddleSize&~7;
        pool->ThreshSize=threshSize;
      }
    }
    return pool;
  }
}

VOID ASM AsmDeletePool(REG(a0,POOL *poolHeader),REG(a6,APTR SysBase))
{ ULONG *pool,size;

  if (((struct Library *)SysBase)->lib_Version>=39)
    DeletePool(poolHeader);
  else if (poolHeader!=NULL) {
    while((pool=(ULONG *)RemHead((struct List *)&poolHeader->PuddleList))!=NULL) {
      size=*--pool; FreeMem(pool,size);
    }
    FreeMem(poolHeader,sizeof(POOL));
  }
}
