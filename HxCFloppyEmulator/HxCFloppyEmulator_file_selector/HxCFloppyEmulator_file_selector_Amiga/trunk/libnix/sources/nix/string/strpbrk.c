#include <string.h>

char *strpbrk(const char *s1,const char *s2)
{ unsigned char *c1=(unsigned char *)s1;
  unsigned char *c2;
  while(*c1!='\0')
  { c2=(unsigned char *)s2;
    while(*c2!='\0')
      if(*c1==*c2++)
        return (char *)c1;
    c1++;
  }
  return (char *)0;
}
