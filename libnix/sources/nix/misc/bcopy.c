#include <stdlib.h>

/* This is a _fast_ block move routine! */

void bcopy(const void *s1,void *s2,size_t n)
{ size_t m;
  if(!n)
    return;
  if(s2<s1)
  { if(n>15)
    { if((long)s1&1)
      { *((char *)s2)++=*((char *)s1)++;
        n--; }
      if(!((long)s2&1))
      { if((long)s1&2)
        { *((short *)s2)++=*((short *)s1)++;
          n-=2; }
        for(m=n/(8*sizeof(long));m;--m)
        { *((long *)s2)++=*((long *)s1)++; *((long *)s2)++=*((long *)s1)++;
          *((long *)s2)++=*((long *)s1)++; *((long *)s2)++=*((long *)s1)++;
          *((long *)s2)++=*((long *)s1)++; *((long *)s2)++=*((long *)s1)++;
          *((long *)s2)++=*((long *)s1)++; *((long *)s2)++=*((long *)s1)++; }
        n&=8*sizeof(long)-1;
        for(m=n/sizeof(long);m;--m)
          *((long *)s2)++=*((long *)s1)++;
        n&=sizeof(long)-1; }
      if(!n) return;
    } do;while(*((char *)s2)++=*((char *)s1)++,--n);
  }else
  { (char *)s1+=n;
    (char *)s2+=n;
    if(n>15)
    { if((long)s1&1)
      { *--((char *)s2)=*--((char *)s1);
        n--; }
      if(!((long)s2&1))
      { if((long)s1&2)
        { *--((short *)s2)=*--((short *)s1);
          n-=2; }
        for(m=n/(8*sizeof(long));m;--m)
        { *--((long *)s2)=*--((long *)s1); *--((long *)s2)=*--((long *)s1);
          *--((long *)s2)=*--((long *)s1); *--((long *)s2)=*--((long *)s1);
          *--((long *)s2)=*--((long *)s1); *--((long *)s2)=*--((long *)s1);
          *--((long *)s2)=*--((long *)s1); *--((long *)s2)=*--((long *)s1); }
        n&=8*sizeof(long)-1;
        for(m=n/sizeof(long);m;--m)
          *--((long *)s2)=*--((long *)s1);
        n&=sizeof(long)-1; }
      if(!n) return;
    } do;while(*--((char *)s2)=*--((char *)s1),--n);
  }
}
