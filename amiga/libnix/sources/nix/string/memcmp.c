#include <string.h>

int memcmp(const void *s1,const void *s2,size_t n)
{ const unsigned char *p1=s1,*p2=s2;
  unsigned long r,c;

  if ((r=n))
    do;while(r=*p1++,c=*p2++,!(r-=c) && --n);
  return r;
}
