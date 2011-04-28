#include <stdio.h>

#undef feof

int feof(FILE *stream)
{ return (stream->flags&__SEOF)!=0; }
