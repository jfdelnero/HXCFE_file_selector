#include <string.h>

extern void bcopy(const void *s1,void *s2,size_t n);

void *memmove(void *s1,const void *s2,size_t n)
{
  bcopy(s2,s1,n); return s1;
}
