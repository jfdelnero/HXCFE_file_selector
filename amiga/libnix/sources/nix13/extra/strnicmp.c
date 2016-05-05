#include <ctype.h>
#include <string.h>
#include "stabs.h"

int strnicmp(const char *s1,const char *s2,size_t len)
{ unsigned char c1,c2;
  int r;

  if ((r=(int)len))
    for(;;) {
      c1=*s1++;
      if (isupper(c1))
        c1+='a'-'A';
      c2=*s2;
      if (isupper(c2))
        c2+='a'-'A';
      if ((r=(char)c1-(char)c2)!=0)
        break;
      if (!*s2++)
        break;
      if (!--len)
        break;
    }
  return r;
}

ALIAS(strncasecmp,strnicmp);
