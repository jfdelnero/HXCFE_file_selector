#include <stdio.h>

int fgetpos(FILE *stream,fpos_t *pos)
{ *pos=ftell(stream); return 0; }
