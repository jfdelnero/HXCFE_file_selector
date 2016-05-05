#include <sys/time.h>

int gettimeofday(struct timeval *tv, struct timezone *tzp)
{
  return 0;
}

/*
 * 2922 is the number of days between 1.1.1970 and 1.1.1978 (2 leap years and 6 normal)
 * 1440 is the number of minutes per day
 *   60 is the number of seconds per minute
 */
