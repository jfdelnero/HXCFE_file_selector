#include <string.h>

void bzero(void *b,size_t n)
{ size_t m;
  if(!n)
    return;
  if(n>15)
  { if((long)b&1)
    { *((char *)b)++=0;
      n--; }
    if((long)b&2)
    { *((short *)b)++=0;
      n-=2; }
    for(m=n/(8*sizeof(long));m;--m)
    { *((long *)b)++=0; *((long *)b)++=0; *((long *)b)++=0; *((long *)b)++=0;
      *((long *)b)++=0; *((long *)b)++=0; *((long *)b)++=0; *((long *)b)++=0; }
    n&=8*sizeof(long)-1;
    for(m=n/sizeof(long);m;--m)
      *((long *)b)++=0;
    if((n&=sizeof(long)-1)==0) return;
  }
  do;while(*((char *)b)++=0,--n);
}
