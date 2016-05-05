#include <string.h>

int strncmp(const char *s1,const char *s2,size_t n)
{ unsigned char *p1=(unsigned char *)s1,*p2=(unsigned char *)s2;
  unsigned long r,c;

  if ((r=n))
    do;while(r=*p1++,c=*p2++,!(r-=c) && (char)c && --n);
  return r;
}
