/* Read a long word into a file -- returns EOF on error, or when the word
   that was read is equal to EOF!!! */

#include <stdio.h>

int getw(FILE *f)
{
  int x;

  if (fread((char *)&x, sizeof(x), 1, f) != 1)
    return EOF;
  else
    return x;
}
