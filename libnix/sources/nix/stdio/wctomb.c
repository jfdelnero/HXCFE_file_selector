#include <stdlib.h>

/* Sorry, the Amiga OS locale currently doesn't support
 * character sets other than ECMA Latin 1, so this is
 * just a "C" locale function 
 */
int wctomb(char *s,wchar_t wc)
{ if(s)
    *s=wc;
  return 1;
}
