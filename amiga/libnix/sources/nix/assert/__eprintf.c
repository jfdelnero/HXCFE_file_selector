#include <stdio.h>
#include <stdarg.h>

void __eprintf(const char *format,...) /* for asserts */
{ va_list args;
  va_start(args,format);
  vfprintf(stderr,format,args);
  va_end(args);
  abort();
}
