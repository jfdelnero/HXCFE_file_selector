#include <string.h>
#include <proto/utility.h>
#include "stabs.h"

int stricmp(const char *s1,const char *s2)
{
  return Stricmp((STRPTR)s1,(STRPTR)s2);
}

ALIAS(strcasecmp,stricmp);
