#include <signal.h>
#include <unistd.h>

extern void (*__signalfunc[])(int);
extern int __signalmask,__signalpending;

#define wlen(s) s,sizeof(s)-1

int raise(int sig)
{ int ret=-1;

  if (!(sig<1||sig>6))
  {
    int *mask=&__signalmask,*pending=&__signalpending;

    *pending|=1<<(sig-1);
    while(!(*mask&(1<<(sig-1)))&&
          (*pending&(1<<(sig-1))))
    {
      *pending&=~(1<<(sig-1));
      if(__signalfunc[sig-1]!=SIG_IGN)
      {
        int oldmask=*mask;
        *mask|=1<<(sig-1);
        if(__signalfunc[sig-1]==SIG_DFL)
        {
          if(sig==SIGINT)
          {
            write(2,wlen("***Break\n")); exit(20);
          }
        }else
          (*__signalfunc[sig-1])(sig);
        *mask=oldmask;
      }
    }
    ret=0;
  }
  return ret;
}
