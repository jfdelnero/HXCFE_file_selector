#include <stdio.h>
#include <errno.h>

int __swbuf(int c,FILE *stream) /* Get next output block */
{ int out,lbs;

  if(stream->flags&(__SSTR|__SERR)) /* sprintf buffer | error on stream */
  { stream->outcount=0;
    errno=EPERM;
    return EOF;
  }else if(stream->flags&__SRD)
  {
    stream->incount=0; /* throw away input buffer */
    stream->tmpp=NULL;
    stream->flags&=~__SRD;
  }
  lbs=stream->flags&__SLBF?-stream->bufsize:0;
  out=(stream->flags&__SNBF?0:stream->bufsize-1)+lbs;
  if(!(stream->flags&__SWR)) /* File wasn't in write mode */
  { stream->p=stream->buffer; /* set buffer */
    stream->outcount=--out;   /* and buffercount */
    stream->flags|=__SWR; }   /* and write mode */
  *stream->p++=c; /* put this character */
  if(stream->outcount<0&&(stream->outcount<lbs||(char)c=='\n'))
  { if(__fflush(stream)) /* Buffer full */
      return EOF;
    stream->p=stream->buffer; /* Set new buffer */
  }
  stream->linebufsize=lbs;
  stream->outcount=out;
  stream->flags|=__SWR;
  return c;
}
