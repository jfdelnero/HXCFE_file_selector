#include <stdlib.h>

/* Sorry, the Amiga OS locale currently doesn't support
 * character sets other than ECMA Latin 1, so this is
 * just a "C" locale function 
 */
size_t mbstowcs(wchar_t *wc,const char *s,size_t n)
{ size_t l=0;
  while(n--&&*s)
  { *wc++=*s++;
    l++; }
  *wc++='\0';
  return l;
}
