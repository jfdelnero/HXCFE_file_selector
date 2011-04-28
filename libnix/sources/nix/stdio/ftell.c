#include <stdio.h>
#include <errno.h>
#include <unistd.h>

long ftell(FILE *stream)
{ long pos;
  if(stream->flags&__SERR) /* Error on stream */
  { errno=EPERM;
    return EOF; }
  if((pos=lseek(stream->file,0,SEEK_CUR))==EOF)
  { stream->flags|=__SERR;
    return pos; }
  if(stream->flags&__SRD)
    pos-=stream->incount+(stream->tmpp!=NULL?stream->tmpinc:0);
  else if(stream->flags&__SWR)
    pos+=stream->p-stream->buffer;
  return pos;
}
