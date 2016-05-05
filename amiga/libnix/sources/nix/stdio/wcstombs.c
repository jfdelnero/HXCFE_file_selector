#include <stdlib.h>

/* Sorry, the Amiga OS locale currently doesn't support
 * character sets other than ECMA Latin 1, so this is
 * just a "C" locale function 
 */
size_t wcstombs(char *s,const wchar_t *wc,size_t n)
{ size_t l=0;
  while(n--&&*wc)
  { *s++=*wc++;
    l++; }
  *s++='\0';
  return l;
}
