#include <stdio.h>

int fputs(const char *s,FILE *stream)
{
  while(*s)
    if(fputc(*s++,stream)==EOF)
      return EOF;
  return 0;
}
