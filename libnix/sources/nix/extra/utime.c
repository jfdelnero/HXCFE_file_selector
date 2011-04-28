#include <sys/time.h>
#include <utime.h>

int utime(const char *file, const struct utimbuf *timep)
{ struct timeval times[2],*t=NULL;

  if (timep) {
    t = &times[0];
    t[0].tv_sec  = timep->actime;
    t[0].tv_usec = 0;
    t[1].tv_sec  = timep->modtime;
    t[1].tv_usec = 0;
  }

  return utimes(file,t);
}
