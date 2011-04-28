#include <stdio.h>
#include <stdarg.h>
#include <string.h>

int snprintf(char *s,size_t size,const char *format,...)
{ int retval;
  va_list args;
  va_start(args,format);
  retval=vsnprintf(s,size,format,args);
  va_end(args);
  return retval;
}
