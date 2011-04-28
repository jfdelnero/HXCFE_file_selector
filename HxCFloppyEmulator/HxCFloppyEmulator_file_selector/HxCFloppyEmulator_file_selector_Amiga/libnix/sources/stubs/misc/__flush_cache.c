#include <exec/execbase.h>
#include <proto/exec.h>

void __flush_cache(APTR adr, ULONG len)
{
  CacheClearE(adr, len, CACRF_ClearD | CACRF_ClearI);
}
