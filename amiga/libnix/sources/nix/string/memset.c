#include <string.h>

void *memset(void *s,int c,size_t n)
{ size_t m;
  if(n)
  { unsigned long *p=(unsigned long *)s;
    if(n>15)
    { c*=0x01010101;
      if((long)p&1)
      { *((char *)p)++=c;
        n--; }
      if((long)p&2)
      { *((short *)p)++=c;
        n-=2; }
      for(m=n/(8*sizeof(long));m;--m)
      { *p++=c; *p++=c; *p++=c; *p++=c; 
        *p++=c; *p++=c; *p++=c; *p++=c; }
      n&=(8*sizeof(long)-1);
      for(m=n/sizeof(long);m;--m)
        *p++=c;
      if((n&=sizeof(long)-1)==0) goto leave;
    }
    do;while(*((char *)p)++=c,--n);
  }
leave:
  return s;
}
