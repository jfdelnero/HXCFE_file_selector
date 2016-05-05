#ifdef __GNUC__
#include <exec/io.h>
#include <exec/devices.h>

void BeginIO(struct IORequest *iorequest)
{ register struct IORequest *a1 __asm("a1")=iorequest;
  register struct Device    *a6 __asm("a6")=iorequest->io_Device;
  __asm volatile ("jsr a6@(-30:W)" :: "r" (a1), "r" (a6));
}
#endif
