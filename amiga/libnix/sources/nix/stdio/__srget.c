#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int __srget(FILE *stream) /* Get next input block */
{
  if(stream->flags&(__SERR|__SEOF)) /* Error on stream / EOF */
  {
    stream->incount=0;
    errno=EPERM;
    return EOF;
  }
  if(stream->flags&__SWR)
  {
    if(__fflush(stream))
      return EOF;
  }else if(stream->tmpp!=NULL) /* File is in ungetc mode */
  {
    stream->p=stream->tmpp;
    stream->incount=stream->tmpinc;
    stream->tmpp=NULL;
    if(--stream->incount>=0)
      return *stream->p++;
  }
  if(stream->flags&__SSTR) /* it's a sscanf buffer */
    return EOF;
  if(stream->flags&(__SNBF|__SLBF)) /* Before reading from line- or unbuffered input file */
  {                   /* fflush all line buffered output files (ANSI) */
    struct filenode *fp=(struct filenode *)__filelist.mlh_Head;
    while(fp->node.mln_Succ)
    {
      if((fp->FILE.flags&(__SWR|__SLBF))==(__SWR|__SLBF))
        __fflush(&fp->FILE); /* Don't return EOF if this fails */
      fp=(struct filenode *)fp->node.mln_Succ;
    }
  }
  stream->flags|=__SRD;
  stream->incount=read(stream->file,stream->buffer,stream->bufsize);
  if(!stream->incount) /* EOF found */
  {
    stream->flags|=__SEOF;
    return EOF;
  }else if(stream->incount<0) /* Error */
  {
    stream->incount=0;
    stream->flags|=__SERR;
    return EOF;
  }
  stream->incount--;
  stream->p=stream->buffer;
  return *stream->p++;
}
