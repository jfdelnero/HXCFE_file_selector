#include <stdio.h>
#include <stdarg.h>

int sprintf(char *s,const char *format,...)
{ int retval;
  va_list args;
  va_start(args,format);
  retval=vsprintf(s,format,args);
  va_end(args);
  return retval;
}
