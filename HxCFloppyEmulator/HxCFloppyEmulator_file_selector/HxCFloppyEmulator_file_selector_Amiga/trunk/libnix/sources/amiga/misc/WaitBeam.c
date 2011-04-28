#include <clib/alib_protos.h>
#include <proto/graphics.h>

VOID waitbeam(LONG pos)
{
  do{}while(pos>VBeamPos());
}
