#include "pool.h"

APTR LibCreatePool(ULONG requirements, ULONG puddleSize, ULONG threshSize)
{
  return AsmCreatePool(requirements,puddleSize,threshSize,*(APTR *)4L);
}

VOID LibDeletePool(APTR poolHeader)
{
  AsmDeletePool(poolHeader,*(APTR *)4L);
}
