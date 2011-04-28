#include "pool.h"

APTR ASM AsmAllocPooled(REG(a0,POOL *poolHeader),REG(d0,ULONG memSize),REG(a6,APTR SysBase))
{
  if (((struct Library *)SysBase)->lib_Version>=39)
    return (AllocPooled(poolHeader,memSize));
  else {

    ULONG *puddle=NULL;

    if (poolHeader!=NULL && memSize!=0) {

      ULONG *p,size;

      if (poolHeader->ThreshSize>memSize) {

        struct MemHeader *a=(struct MemHeader *)poolHeader->PuddleList.mlh_Head;

        for(;;) {
          if (a->mh_Node.ln_Succ!=NULL) {
            if (a->mh_Node.ln_Type && (puddle=(ULONG *)Allocate(a,memSize))!=NULL)
              break;
            a=(struct MemHeader *)a->mh_Node.ln_Succ; continue;
          }

          size=poolHeader->PuddleSize+sizeof(struct MemHeader)+2*sizeof(ULONG);
          if ((puddle=(ULONG *)AllocMem(size,poolHeader->MemoryFlags))==NULL)
            goto out; /* why is gcc so dumb ??? */

          *puddle++=size;
          a=(struct MemHeader *)puddle;
          a->mh_Node.ln_Type=NT_MEMORY;
          a->mh_Lower=a->mh_First=(struct MemChunk *)((UBYTE *)a+sizeof(struct MemHeader)+sizeof(UBYTE *));
          a->mh_First->mc_Next=NULL;
          a->mh_Free=a->mh_First->mc_Bytes=poolHeader->PuddleSize;
          a->mh_Upper=(char *)a->mh_First+a->mh_Free;
          AddHead((struct List *)&poolHeader->PuddleList,&a->mh_Node);
          puddle=(ULONG *)Allocate(a,memSize);
          break;
        }

        if (poolHeader->MemoryFlags&MEMF_CLEAR) {
          p=puddle; memSize+=7; memSize>>=3;
          do { *p++=0; *p++=0; } while(--memSize);
        }
      }
      else {

        size=memSize+sizeof(struct MinNode)+2*sizeof(ULONG);

        if ((puddle=(ULONG *)AllocMem(size,poolHeader->MemoryFlags))!=NULL) {
          *puddle++=size;
          AddTail((struct List *)&poolHeader->PuddleList,(struct Node *)puddle);
          puddle=(ULONG *)((struct MinNode *)puddle+1);
          *puddle++=0;
        }
      }
    }
out:
    return puddle;
  }
}
