#include <stdio.h>
#include <stdarg.h>

int scanf(const char *format,...)
{ int retval;
  va_list args;
  va_start(args,format);
  retval=vscanf(format,args);
  va_end(args);
  return retval;
}
