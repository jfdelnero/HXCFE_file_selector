#include <devices/inputevent.h>
#include <proto/exec.h>

VOID FreeIEvents(struct InputEvent *events)
{ APTR SysBase = *(APTR *)4L;
  struct InputEvent *next;
 
  while (events != NULL) {
    next = events->ie_NextEvent;
    FreeMem(events,sizeof(*events));
    events = next;
  }
}
