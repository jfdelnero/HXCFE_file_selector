#include <string.h>
#include "stabs.h"

char *strchr(const char *s,int c)
{
  while (*s!=(char)c)
    if (!*s++)
      { s = (char *)0; break; }
  return (char *)s;
}

ALIAS(index,strchr);
