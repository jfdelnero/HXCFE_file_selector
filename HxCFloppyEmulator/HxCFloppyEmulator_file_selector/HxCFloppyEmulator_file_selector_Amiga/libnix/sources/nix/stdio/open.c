#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#define DEVICES_TIMER_H
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include "stabs.h"

/*
**
*/
extern void __seterrno(void);

/*
**
*/
static StdFileDes **stdfiledes;
static unsigned long stdfilesize=0;
static long stderrdes=0; /* The normal Amiga shell sets no process->pr_CES stream -
                          * we use Open("*",MODE_NEWFILE) in this case
                          */

/*
**
*/
static void _setup_file(StdFileDes *fp)
{ fp->lx_inuse  = 1;
  fp->lx_isatty = 0;//IsInteractive(fp->lx_fh) ? -1 : 0;
}

/*
**
*/
static __inline StdFileDes *_allocfd(void)
{ StdFileDes *fp,**sfd;
  int file,max;

  for(sfd=stdfiledes,max=stdfilesize,file=0;file<max;sfd++,file++)
    if(!sfd[0] || !sfd[0]->lx_inuse)
      break;

  if(file>SHRT_MAX)
  { errno=EMFILE;
    return NULL;
  }

  if(file==max)
  { if((sfd=realloc(stdfiledes,(file+1)*sizeof(fp)))==NULL)
    { errno=ENOMEM;
      return NULL;
    }
    stdfiledes=sfd;
    stdfilesize++;
    *(sfd=&sfd[file]) = 0;
  }

  if((fp=sfd[0])==NULL)
  { if((sfd[0]=fp=malloc(sizeof(*fp)))==NULL)
    { errno=ENOMEM;
      return NULL;
    }
    fp->lx_pos = file;
  }

  return fp;
}

/*
**
*/
int open(const char *path,int flags,...)
{
  return -1;
}

int close(int d)
{
  return 0;
}

ssize_t read(int d,void *buf,size_t nbytes)
{
  return EOF;
}

ssize_t write(int d,const void *buf,size_t nbytes)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd) {
    long r;
    __chkabort();
    switch((sfd->lx_oflags&O_APPEND)!=0) {
      case 1:
        if(!sfd->lx_isatty&&(Seek(sfd->lx_fh,0,OFFSET_END)==EOF))
          break;
      default:
        if((r=Write(sfd->lx_fh,(char *)buf,nbytes))!=EOF)
          return r;
    }
    __seterrno();
  }

  return EOF;
}

off_t lseek(int d,off_t offset,int whence)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd) {
    long r,file=sfd->lx_fh;
    __chkabort();
    if (Seek(file,offset,whence==SEEK_SET?OFFSET_BEGINNING:
                         whence==SEEK_END?OFFSET_END:OFFSET_CURRENT)!=EOF)
      if ((r=Seek(file,0,OFFSET_CURRENT))!=EOF)
        return r;
    __seterrno();
  }

  return EOF;
}

int isatty(int d)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  return sfd?sfd->lx_isatty:0;
}

/*
**
*/
int _lx_addflags(int d,int oflags)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  return sfd?sfd->lx_oflags|=oflags:0;
}

/*
** convert fd to a StdFileDes
*/
StdFileDes *_lx_fhfromfd(int d)
{ if(d<(int)stdfilesize)
  { StdFileDes *sfd=stdfiledes[d];
    if(sfd&&sfd->lx_inuse)
      return sfd; }
  return NULL;
}

/*
**
*/
void __initstdio(void)
{ extern struct WBStartup *_WBenchMsg;
  StdFileDes *fp,**sfd;

  if((stdfiledes=sfd=(StdFileDes **)malloc(3*sizeof(StdFileDes *)))) {
    if((sfd[STDIN_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
      fp->lx_fh     = Input();
      fp->lx_pos    = STDIN_FILENO;
      fp->lx_sys    = -1;
      fp->lx_oflags = O_RDONLY;
      _setup_file(fp);
      if((sfd[STDOUT_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
        fp->lx_fh     = Output();
        fp->lx_pos    = STDOUT_FILENO;
        fp->lx_sys    = -1;
        fp->lx_oflags = O_WRONLY;
        _setup_file(fp);
        if((sfd[STDERR_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
      //    if((fp->lx_fh=((struct Process *)FindTask(NULL))->pr_CES)==0)
        //    if(_WBenchMsg||(fp->lx_fh=stderrdes=Open("*",MODE_OLDFILE))==0)
          //    fp->lx_fh=sfd[STDOUT_FILENO]->lx_fh;
          fp->lx_pos    = STDERR_FILENO;
          fp->lx_sys    = -1;
          fp->lx_oflags = O_WRONLY;
          _setup_file(fp);
          stdfilesize += 3; return;
        }
      }
    }
  }
  exit(20);
}
ADD2INIT(__initstdio,-30);

void __exitstdio(void)
{ int i,max;

  for(max=stdfilesize,i=0;i<max;i++) {
    StdFileDes *sfd = stdfiledes[i];
    if(sfd && sfd->lx_inuse) {
      close(i);
    }
  }

  if(stderrdes)
    Close(stderrdes);
}
ADD2EXIT(__exitstdio,-30);
