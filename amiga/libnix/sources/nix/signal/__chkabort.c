#include <signal.h>
#include <proto/exec.h>
#include <dos/dos.h>

void __chkabort(void)
{
  if (SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
    raise(SIGINT);
}
