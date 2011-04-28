/* Write a long word to a file */

#include <stdio.h>

int putw(int w, FILE *f)
{
  if (fwrite((char *)&w, sizeof(w), 1, f) != 1)
    return EOF;
  else
    return 0;
}
