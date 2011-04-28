#include "pool.h"

APTR LibAllocPooled(APTR poolHeader, ULONG memSize)
{
  return AsmAllocPooled(poolHeader,memSize,*(APTR *)4L);
}
