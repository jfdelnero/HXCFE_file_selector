#include <time.h>

extern long __gmtoffset;

struct tm *localtime(const time_t *t)
{ time_t ti=*t;
  ti-=__gmtoffset*60;
  return gmtime(&ti);
}
