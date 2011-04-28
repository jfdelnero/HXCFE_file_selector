#include <string.h>

char *strstr(const char *s1,const char *s2)
{ const char *c1,*c2;

  do {
    c1 = s1; c2 = s2;
    while(*c1 && *c1==*c2) {
      c1++; c2++;
    }
    if (!*c2)
      return (char *)s1;
  } while(*s1++);
  return (char *)0;
}
