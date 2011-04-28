#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int __fflush(FILE *stream) /* fflush exactly one file */
{ unsigned char *subbuf;
  long size,subsize;

  if(stream->flags&__SERR) /* Error on stream */
  { errno=EPERM;
    return EOF; }
  if(stream->flags&__SWR) /* Works only on output streams */
  { size=stream->p-stream->buffer; /* calculate size */
    subbuf=stream->buffer;
    while(size)
    { if((subsize=write(stream->file,subbuf,size))<0)
      { stream->flags|=__SERR; /* error flag */
        return EOF; }
      size-=subsize;
      subbuf+=subsize;
    }
    stream->flags&=~__SWR; /* unset write state */
    stream->outcount=0;
    stream->linebufsize=0;
  } /* Nothing to be done for input streams */
  return 0;
}
