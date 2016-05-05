#include <dos/dos.h>
#include <proto/dos.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

extern void __seterrno(void);

FILE *freopen(const char *filename,const char *mode,FILE *stream)
{ int error=__fflush(stream);

  close(stream->file);
  if(stream->name!=NULL) /* file is temporary */
  { BPTR cd=CurrentDir(stream->tmpdir); /* cd t: */
    if(!DeleteFile(stream->name))  /* delete file */
    { __seterrno();
      error=1; }
    free(stream->name); /* free filename */
    stream->name=NULL;
    UnLock(CurrentDir(cd)); /* cd back, unlock t: */
  }
  stream->file=0;

  if(error)
    return NULL;

  if(filename!=NULL)
  { long file,flags=O_RDONLY;
    char ch;
    if(mode==NULL)
      return NULL;
    if(ch=*mode++,ch!='r')
      if(flags=O_WRONLY|O_CREAT|O_TRUNC,ch!='w')
        if(flags=O_WRONLY|O_CREAT|O_APPEND,ch!='a')
          return NULL;
    if((ch=*mode++))
    { if(ch=='+')
      { if((ch=*mode++) && (ch!='b'||*mode))
          return NULL;
        flags=(flags&~O_ACCMODE)|O_RDWR; }
      else if(ch!='b')
        return NULL;
      else if((ch=*mode++))
      { if(ch!='+'||*mode)
          return NULL;
        flags=(flags&~O_ACCMODE)|O_RDWR; }
    }

    if((file=open(filename,flags,0777))<0)
      return NULL;

    if(flags&O_APPEND)
      (void)lseek(file,0,SEEK_END);

    /* clear a lot of flags */
    stream->flags&=~(__SWO|__SERR|__SEOF|__SWR|__SRD|__SNBF|__SLBF);
    if(flags&O_WRONLY)
      stream->flags|=__SWO; /* set write-only flag */
    if(isatty(file))
      stream->flags|=__SLBF; /* set linebuffered flag */
    stream->file=file;
  }

  return stream;
}
