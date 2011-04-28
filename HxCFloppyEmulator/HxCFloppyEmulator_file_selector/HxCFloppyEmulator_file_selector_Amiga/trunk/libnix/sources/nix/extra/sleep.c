#include <devices/timer.h>

extern void dotimer(ULONG,ULONG,struct timeval *);

void sleep(int secs)
{ struct timeval tv;

  if ((int)(tv.tv_secs=secs)>0)
  { tv.tv_micro=0;
    dotimer(UNIT_VBLANK,TR_ADDREQUEST,&tv);
  }
}
