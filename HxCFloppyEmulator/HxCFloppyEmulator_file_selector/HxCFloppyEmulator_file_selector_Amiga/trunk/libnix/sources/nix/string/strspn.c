#include <string.h>

size_t strspn(const char *s1,const char *s2)
{ size_t i;
  unsigned char *c1=(unsigned char *)s1;
  unsigned char *c2;
  for(i=0;;i++)
  { c2=(unsigned char *)s2;
    while(*c2!='\0'&&c1[i]!=*c2)
      c2++;
    if(*c2=='\0')
      return i;
  }
}
