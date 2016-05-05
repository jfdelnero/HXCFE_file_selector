#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strsup.h>

int vsscanf(const char *s,const char *format,va_list args)
{ FILE buffer;
  buffer.p=(char *)s;
  buffer.incount=strlen(s);
  buffer.outcount=0;
  buffer.flags=__SSTR|__SRD;
  buffer.tmpp=NULL;
  return vfscanf(&buffer,format,args);
}
