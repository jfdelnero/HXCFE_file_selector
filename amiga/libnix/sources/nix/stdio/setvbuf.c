#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int setvbuf(FILE *stream,char *buf,int mode,size_t size)
{ short flags=stream->flags&~(__SNBF|__SLBF);
  if(mode==_IONBF)
    flags|=__SNBF;
  else if(mode==_IOLBF)
    flags|=__SLBF;
  if(size!=(size_t)stream->bufsize||buf!=(char *)stream->buffer)
  { if(__fflush(stream))
      return -1;
    stream->incount=0;
    stream->flags=(flags&=~__SRD);
    stream->tmpp=0;
    mode=(buf==NULL);
    if(mode&&(buf=malloc(size=size?size:1))==NULL)
    { errno=ENOMEM;
      return -1; }
    if(flags&__SMBF)
      free(stream->buffer);
    if(mode)
      flags|=__SMBF;
    else
      flags&=~__SMBF;
    stream->buffer=buf;
    stream->bufsize=size;
  } /* Need not adjust outcount, since setvbuf affects only the NEXT full buffer */
  stream->flags=flags; return 0;
}
