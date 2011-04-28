#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#define DEVICES_TIMER_H
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "debuglib.h"
#include "select.h"
#include "stdio.h"
#include "stabs.h"

/*
**
*/
extern void __seterrno(void);

/*
**
*/
struct MsgPort *__selport=0;

/*
**
*/
static StdFileDes **stdfiledes;
static unsigned long stdfilesize=0;
static long stderrdes=0; /* The normal Amiga shell sets no process->pr_CES stream
                          * -> we use Open("*",MODE_NEWFILE) in this case */

/*
**
*/
static ssize_t _file_read(StdFileDes *sfd,void *buf,size_t nbytes)
{ long r;
  __chkabort();
  if ((r=Read(sfd->lx_fh,buf,nbytes))!=EOF)
    return r;
  __seterrno(); return EOF;
}

static ssize_t _file_write(StdFileDes *sfd,const void *buf,size_t nbytes)
{ long r;
  __chkabort();
  switch((sfd->lx_oflags&O_APPEND)!=0) {
    case 1:
      if(!sfd->lx_isatty&&(Seek(sfd->lx_fh,0,OFFSET_END)==EOF))
        break;
    default:
      if((r=Write(sfd->lx_fh,(char *)buf,nbytes))!=EOF)
        return r;
  }
  __seterrno(); return EOF;
}

static int _file_close(StdFileDes *sfd)
{
  /* this is the synchronous way of dealing with packets that may
   * arrive at a port.
   */
  if (SELPKT_IN_USE(sfd))
    for (;;) {
      struct StandardPacket *pkt = GetPacket(__selport);
      if (pkt) {
        DB( BUG("Got packet %lx, sp is %lx\n", pkt, sfd->lx_packet); )
        pkt->sp_Pkt.dp_Port = NULL;
        if (pkt == sfd->lx_packet)
          break;
      }
      else {
        DB( BUG("Waiting for select packet\n"); )
        Wait(1<<__selport->mp_SigBit);
      }
    }

  FreeDosObject(DOS_STDPKT,sfd->lx_packet);

  if (!sfd->lx_sys&&!Close(sfd->lx_fh)) {
    __seterrno(); return EOF;
  }

  return 0;
}

static int _file_dup(StdFileDes *sfd)
{ extern StdFileDes *_allocfd(void);
  StdFileDes *fp2 = _allocfd();

  if (fp2) {
    int fd = fp2->lx_pos;
    fp2->lx_inuse = 0;
    stdfiledes[fd] = sfd;
    sfd->lx_inuse++;
    return fd;
  }

  return -1;
}

static int _file_fstat(StdFileDes *fp,struct stat *buf)
{ extern int __stat(struct stat *buf,struct FileInfoBlock *fib);
  struct FileInfoBlock *fib;
  long pos,len,fh;

  if ((fh=fp->lx_fh)) {
    if ((fib=(struct FileInfoBlock *)AllocDosObject(DOS_FIB,NULL)) == NULL) {
      __seterrno(); return -1;
    }
  } else return -1;

  memset(buf,0,sizeof(*buf));

  buf->st_mode    = S_IFCHR | 0777;
  buf->st_nlink   = 1;
  buf->st_blksize = 512;
  /*buf->st_blocks  = 0;*/

  if (((struct FileHandle *)BADDR(fh))->fh_Type) {

    if (!ExamineFH(fh,fib)) {

      len = 0; pos = Seek(fh,0,OFFSET_END);
      if (pos >= 0 && (IoErr() != ERROR_ACTION_NOT_KNOWN))
        len = Seek(fh,pos,OFFSET_BEGINNING);

      fib->fib_DiskKey      = (ino_t)~((LONG)fh);
      fib->fib_DirEntryType = -1;
      fib->fib_Size         = len;
      fib->fib_Protection   = 0xff0;
      fib->fib_NumBlocks    = (len+=511,len>>=9);
      DateStamp(&fib->fib_Date);
    }

    __stat(buf,fib);

    if (Seek(fh,0,OFFSET_CURRENT),IoErr())
      buf->st_mode = ((buf->st_mode & ~S_IFREG) | S_IFCHR);
  }

  FreeDosObject(DOS_FIB,fib); return 0;
}

static int _file_select(StdFileDes *sfd,int select_cmd,int io_mode,fd_set *fd,u_long *sigs)
{ int result;

  DB( BUG("file_select %ld, %ld for fd %ld\n", select_cmd, io_mode, sfd->lx_pos); )
  switch (select_cmd)  {
    case SELCMD_PREPARE:
      if (!SELPKT_IN_USE(sfd)) {
        DB( BUG("Sending WAIT_FOR_CHAR packet\n"); )
        SelSendPacket1(sfd, __selport, ACTION_WAIT_CHAR, 10 * 1000000);
      }
      DB( BUG("Prepare for fd %ld is 0x%lx\n", sfd->lx_pos, (1 << __selport->mp_SigBit)); )
      return (1 << __selport->mp_SigBit);

    case SELCMD_CHECK:
      /* only read is supported, other modes default to `ok' */
      if (io_mode != SELMODE_IN)
        return 1;

      if (SELPKT_IN_USE(sfd) /*IsMsgPortEmpty(__selport)*/) {
        DB( BUG("Check: packet in use\n"); )
        return 0;
      }

      (void)GetPacket(__selport);
      /* there are two possible answers: error (packet not supported)
      * and the `real' answer.
      * An error is treated as to allow input, so select() won't block
      * indefinitely...
      * & 1 converts dos-true (-1) into normal true (1) ;-)
      */
      DB( BUG("LastError is %ld, LastResult is %ld\n", SelLastError(sfd), SelLastResult(sfd)); )

      result = SelLastError(sfd);
        /*(SelLastError(sfd) != 0) ? 1 : (SelLastResult(sfd) & 1);*/
      /* don't make __write() think its last packet failed.. */
      SelLastError(sfd) = 0;
      sfd->lx_packet->sp_Pkt.dp_Port = NULL; /* packet not being used anymore */

      DB( BUG("check result for fd %ld is %ld\n", sfd->lx_pos, result); )
      return result;

    case SELCMD_POLL:
      if (io_mode != SELMODE_IN)
        return 1;
      result = !sfd->lx_isatty || WaitForChar(sfd->lx_fh, 0);
      DB( BUG("Poll result for fd %ld is %ld\n", sfd->lx_pos, result); )
      return result;
  }
  /*NOTREACHED*/
  return 0;
}

static void *_setup_file(StdFileDes *fp)
{
  fp->lx_type   = LX_FILE;
  fp->lx_inuse  = 1;
  fp->lx_isatty = IsInteractive(fp->lx_fh) ? -1 : 0;
  fp->lx_packet = AllocDosObject(DOS_STDPKT,NULL);
  fp->lx_read   = _file_read;
  fp->lx_write  = _file_write;
  fp->lx_fstat  = _file_fstat;
  fp->lx_close  = _file_close;
  fp->lx_dup    = _file_dup;
  fp->lx_select = _file_select;

  return fp->lx_packet;
}

/*
**
*/
StdFileDes *_allocfd(void)
{ StdFileDes *fp,**sfd;
  int file,max;

  for(sfd=stdfiledes,max=stdfilesize,file=0;file<max;sfd++,file++)
    if(!sfd[0] || !sfd[0]->lx_inuse)
      break;
  DB( BUG("Allocfd: file is %ld, max is %ld\n", file, max); )

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
  DB( BUG("Allocfd returning fd %ld\n", fp->lx_pos); )

  return fp;
}

/*
**
*/
int open(const char *path,int flags,...)
{ extern char *__amigapath(const char *path);
  StdFileDes *sfd;

#ifdef IXPATHS
  if((path=__amigapath(path))==NULL)
    return -1;
#endif

  if ((sfd=_allocfd())) {
    sfd->lx_sys=0;
    sfd->lx_oflags=flags;
    if ((sfd->lx_fh=Open((char *)path,flags&O_TRUNC?MODE_NEWFILE:
                         flags&(O_WRONLY|O_RDWR)?MODE_READWRITE:MODE_OLDFILE))) {
      if (_setup_file(sfd))
        return sfd->lx_pos;
    }
    __seterrno();
    if (sfd->lx_fh) Close(sfd->lx_fh);
    sfd->lx_inuse = 0;
  }

  return -1;
}

int close(int d)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  DB( BUG("Close: FD %ld, inuse is %ld\n", d, sfd->lx_inuse); )
  if (sfd) {
    DB( BUG("close %ld, inuse is %ld\n", d, sfd->lx_inuse); )
    if (!(sfd->lx_inuse-=1)) {
      sfd->lx_pos=d; return sfd->lx_close(sfd);
    }
    else {
      stdfiledes[d] = 0;
    }
  }

  return 0;
}

ssize_t read(int d,void *buf,size_t nbytes)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd) {
    return sfd->lx_read(sfd, buf, nbytes);
  }
  else {
    errno = EBADF;
    return -1;
  }
}

ssize_t write(int d,const void *buf,size_t nbytes)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd) {
    return sfd->lx_write(sfd, buf, nbytes);
  }
  else {
    errno = EBADF;
    return -1;
  }
}

off_t lseek(int d,off_t offset,int whence)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  if (sfd && sfd->lx_type==LX_FILE) {
    long r,file=sfd->lx_fh;
    __chkabort();
    if (Seek(file,offset,whence==SEEK_SET?OFFSET_BEGINNING:
                         whence==SEEK_END?OFFSET_END:OFFSET_CURRENT)!=EOF)
      if ((r=Seek(file,0,OFFSET_CURRENT))!=EOF)
        return r;
    __seterrno();
  }
  else {
    errno = EBADF;
  }

  return -1;
}

int isatty(int d)
{ StdFileDes *sfd = _lx_fhfromfd(d);
  if (sfd && sfd->lx_type==LX_FILE) {
    return sfd->lx_isatty;
  }
  else {
    errno = EBADF;
    return 0;
  }
}

int dup2(int d1,int d2)
{
  if (d1 != d2) {

    StdFileDes *sfd2 = _lx_fhfromfd(d2);
    StdFileDes *sfd1 = _lx_fhfromfd(d1);

    DB( BUG("dup2 %ld %ld\n", d1, d2); )

    if (!sfd1 || (d2 > (int)stdfilesize)) {
      errno=EBADF; return -1;
    }

    if (sfd2) {
      close(d2);
      //if (!sfd2->lx_inuse)
      //  free(sfd2);
    }

    stdfiledes[d2] = stdfiledes[d1];
    stdfiledes[d1]->lx_inuse++;

    return d2;
  }

  return d1;
}

/*
**
*/
int _lx_addflags(int d,int oflags)
{ StdFileDes *sfd = _lx_fhfromfd(d);

  return sfd&&sfd->lx_type==LX_FILE?sfd->lx_oflags|=oflags:0;
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

#if 0
/*
** convert StdFileDes->lx_sock to StdFileDes
*/
StdFileDes *_lx_fdfromfh(int sock,LX_FILE_TYPE type)
{ int i;
  for (i = 0; i < stdfilesize; i++) {
    StdFileDes *sfd = stdfiledes[i];
    if (sfd->lx_inuse && sfd->lx_type == type) {
      switch (sfd->lx_type) {
        case LX_SOCKET:
          if (sfd->lx_sock == sock)
            return sfd;
          break;

        case LX_FILE:
          if (sfd->lx_fh == sock)
            return sfd;
          break;
      }
    }
  }
  return NULL;
}
#endif

/*
**
*/
void __initstdio(void)
{ extern struct WBStartup *_WBenchMsg;
  StdFileDes *fp,**sfd;

  if((__selport=CreateMsgPort())) {
    if((stdfiledes=sfd=(StdFileDes **)malloc(3*sizeof(StdFileDes *)))) {
      if((sfd[STDIN_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
        fp->lx_pos    = STDIN_FILENO;
        fp->lx_sys    = -1;
        fp->lx_oflags = O_RDONLY;
        fp->lx_fh     = Input();
        if(_setup_file(fp)) {
          ++stdfilesize;
          if((sfd[STDOUT_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
            fp->lx_pos    = STDOUT_FILENO;
            fp->lx_sys    = -1;
            fp->lx_oflags = O_WRONLY;
            fp->lx_fh     = Output();
            if(_setup_file(fp)) {
              ++stdfilesize;
              if((sfd[STDERR_FILENO]=fp=(StdFileDes *)malloc(sizeof(StdFileDes)))) {
                fp->lx_pos    = STDERR_FILENO;
                fp->lx_sys    = -1;
                fp->lx_oflags = O_WRONLY;
                if((fp->lx_fh=((struct Process *)FindTask(NULL))->pr_CES)==0)
                  if(_WBenchMsg||(fp->lx_fh=stderrdes=Open("*",MODE_OLDFILE))==0)
                    fp->lx_fh=sfd[STDOUT_FILENO]->lx_fh;
                if(_setup_file(fp)) {
                  ++stdfilesize; return;
                }
              }
            }
          }
        }
      }
    }
  }

  exit(20);
}
//ADD2INIT(__initstdio,-30);

void __exitstdio(void)
{ int i,max;

  DB( BUG("In __exitstdio max # of fds are %ld\n", stdfilesize); )

  for(max=stdfilesize,i=0;i<max;i++) {
    StdFileDes *sfd = stdfiledes[i];
    if (sfd && sfd->lx_inuse) {
      DB( BUG("Closing file %ld\n", i); )
      DB( BUG("sfd is %lx\n", sfd); )
      DB( BUG("Type is %ld\n", sfd->lx_type); )
      DB( BUG("inuse is %ld\n", sfd->lx_inuse); )
      close(i);
    }
  }
  DB( BUG("Done closing files\n"); )

  if(stderrdes) {
    DB( BUG("Closing stderrdes"); )
    Close(stderrdes);
  }

  if(__selport) {
    DeleteMsgPort(__selport);
  }
}
//ADD2EXIT(__exitstdio,-30);
