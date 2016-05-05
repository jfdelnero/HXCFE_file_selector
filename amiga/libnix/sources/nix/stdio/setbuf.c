#include <stdio.h>

int setbuf(FILE *stream,char *buf)
{ return setvbuf(stream,buf,buf?_IOFBF:_IONBF,BUFSIZ); }
