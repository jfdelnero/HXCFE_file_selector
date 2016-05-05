#include <stdlib.h>

/* Sorry, the Amiga OS locale currently doesn't support
 * character sets other than ECMA Latin 1, so this is
 * just a "C" locale function 
 */
int mblen(const char *s,size_t n)
{ size_t l=0;
  while(n--&&*s++)
    l++;
  return sizeof(wchar_t)*(l+1); /* 1 for the NUL character */
}
