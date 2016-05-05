#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <string.h>
#include <strsup.h>
#include "stabs.h"

extern void timer(long *);

/*
** store system time at startup. getrusage will return
** (system time - time at startup) as the user time.
*/
   
static long initclock[2];

void __inittimer()
{
  timer(initclock);
}

//ADD2INIT(__inittimer,1);

int getrusage(int who, struct rusage *rusage)
{
  static struct rusage r0 =
    { {0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  long clock[2];
  int status = 0;

  switch(who) {
    case RUSAGE_SELF:
      timer(clock);
      clock[0] -= initclock[0];
      clock[1] -= initclock[1];
      if (clock[1] < 0) {
        clock[1] += 1000000;
        clock[0] --;
      }
      memcpy(rusage,&r0,sizeof(struct rusage));
      rusage->ru_utime.tv_sec = clock[0];
      rusage->ru_utime.tv_usec = clock[1];
      break;
    case RUSAGE_CHILDREN:
      memcpy(rusage,&r0,sizeof(struct rusage));
      break;
    default:
      errno = EINVAL;
      status = -1;
  }
  return status;
}
