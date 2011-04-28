#include <stdio.h>

int setlinebuf(FILE *stream)
{ return setvbuf(stream,NULL,_IOLBF,0); }
