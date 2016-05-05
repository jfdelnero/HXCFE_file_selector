/*
 * Get next token from string *stringp, where tokens are nonempty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strsep returns NULL.
 */

#include <string.h>

char *strsep(char **stringp, const char *delim)
{ char *s,*tok;

  if ((tok=*stringp))
    for (s=tok;;) {
      const char *spanp=delim;
      int sc,c=*s++;
      do {
        if ((sc = *spanp++) == c) {
          if (c)
            s[-1] = 0;
          else
            s = NULL;
          tok = *stringp;
          *stringp = s;
          return tok;
        }
      } while (sc != 0);
    }
  return tok;
}
