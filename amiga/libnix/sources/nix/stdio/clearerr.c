#include <stdio.h>

void clearerr(FILE *stream)
{ stream->flags&=~(__SERR|__SEOF); }
