#include <string.h>

size_t strcspn(const char *s1,const char *s2)
{ unsigned char *c2,*c1=(unsigned char *)s1;
  size_t i;
  for(i=0;;i++) {
    c2=(unsigned char *)s2;
    do
      if(c1[i]==*c2)
        return i;
    while(*c2++!='\0');
  }
}
