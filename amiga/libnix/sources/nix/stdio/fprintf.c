#include <stdio.h>
#include <stdarg.h>

int fprintf(FILE *stream,const char *format,...)
{ int retval;
  va_list args;
  va_start(args,format);
  retval=vfprintf(stream,format,args);
  va_end(args);
  return retval;
}
