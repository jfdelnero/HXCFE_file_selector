#include "pool.h"

VOID ASM AsmFreePooled(REG(a0,POOL *poolHeader),REG(a1,APTR memory),REG(d0,ULONG memSize),REG(a6,APTR SysBase))
{
  if (((struct Library *)SysBase)->lib_Version>=39)
    return (FreePooled(poolHeader,memory,memSize));
  else if (poolHeader!=NULL && memory!=NULL) {

    ULONG size,*puddle=(ULONG *)((struct MinNode *)memory-1)-1;

    if (poolHeader->ThreshSize>memSize) {

      struct MemHeader *a=(struct MemHeader *)&poolHeader->PuddleList.mlh_Head;

      for(;;) {
        a=(struct MemHeader *)a->mh_Node.ln_Succ;
        if (a->mh_Node.ln_Succ==NULL)
          return;
        if (a->mh_Node.ln_Type && memory>=a->mh_Lower && memory<a->mh_Upper)
          break;
      }
      Deallocate(a,memory,memSize);
      if (a->mh_Free!=poolHeader->PuddleSize)
        return;
      puddle=(ULONG *)&a->mh_Node;
    }
    Remove((struct Node *)puddle);
    size=*--puddle; FreeMem(puddle,size);
  }
}
