#include <string.h>

int strcmp(const char *s1,const char *s2)
{ unsigned char *p1=(unsigned char *)s1, *p2=(unsigned char *)s2;
  unsigned long r,c;

  do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c); return r;
}
