#include <stdio.h>
#include <stdarg.h>

int vsnprintf(char *s,size_t size,const char *format,va_list args)
{ if ((int)size>0) {
    int retval;
    FILE buffer;
    buffer.p=s;
    buffer.incount=0;
    buffer.outcount=size-1;
    buffer.flags=__SSTR|__SWR;
    buffer.linebufsize=0;
    retval=vfprintf(&buffer,format,args);
    buffer.outcount++;
    fputc('\0',&buffer);
    return retval;
  }
  return EOF;
}
