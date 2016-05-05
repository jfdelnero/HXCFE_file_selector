#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>

clock_t times(struct tms *tp)
{ struct rusage r;
  time_t tim;

  if (getrusage(RUSAGE_SELF, &r) < 0)
    return (clock_t)-1;

  tp->tms_utime = r.ru_utime.tv_sec * CLK_TCK +
		  (r.ru_utime.tv_usec * CLK_TCK) / 1000000;
  tp->tms_stime = r.ru_stime.tv_sec * CLK_TCK +
  		  (r.ru_stime.tv_usec * CLK_TCK) / 1000000;

  if (getrusage(RUSAGE_CHILDREN, &r) < 0)
    return (clock_t)-1;

  tp->tms_cutime = 0;
  tp->tms_cstime = 0;

  if ((tim=time(NULL)) == (time_t)-1)
    return (clock_t)-1;

  /* The value we are supposed to return does not fit in 32 bits.
     Still, it is useful if you are interested in time differences
     in CLK_TCKths of a second.
  */
  return tim * CLK_TCK;
}
