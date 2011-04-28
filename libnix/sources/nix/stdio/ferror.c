#include <stdio.h>

#undef ferror

int ferror(FILE *stream)
{ return (stream->flags&__SERR)!=0; }
