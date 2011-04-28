#include <devices/timer.h>
#include <unistd.h>

extern void dotimer(ULONG,ULONG,struct timeval *);

void usleep(unsigned int tim)
{ struct timeval tv;

  tv.tv_secs = tim / 1000000;
  tv.tv_micro = tim % 1000000;

  dotimer(UNIT_VBLANK,TR_ADDREQUEST,&tv);
}
