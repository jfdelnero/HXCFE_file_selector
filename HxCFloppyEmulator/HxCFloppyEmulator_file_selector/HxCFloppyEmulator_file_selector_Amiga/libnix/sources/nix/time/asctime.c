#include <time.h>

static char buffer[26];

char *asctime(const struct tm *t)
{ strftime(buffer,sizeof(buffer),"%C\n",t);
  return buffer;
}

char *ctime(const time_t *t)
{ return asctime(localtime(t)); }
