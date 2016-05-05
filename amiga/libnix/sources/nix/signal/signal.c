#include <signal.h>

extern void (*__signalfunc[])(int);
extern int __signalmask,__signalpending;

void (*signal(int sig,void (*func)(int)))(int)
{
  void (*oldfunc)(int);
  if(sig<1||sig>6)
    return SIG_ERR;
  oldfunc=__signalfunc[sig-1];
  __signalfunc[sig-1]=func;
  __signalmask&=~(1<<(sig-1));
  if(__signalpending&(1<<(sig-1)))
    raise(sig);
  return oldfunc;
}
