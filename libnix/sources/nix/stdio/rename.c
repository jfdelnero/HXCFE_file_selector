#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <proto/dos.h>

extern void __seterrno(void);
extern char *__amigapath(const char *path);

int rename(const char *old,const char *new)
{ int ret=-1;

#ifdef IXPATHS
  if((old=__amigapath(old))!=NULL)
  { 
    if((old=strdup(old))!=NULL)
    { 
      if((new=__amigapath(new))!=NULL)
      {
#endif

#if defined (__GNUC__)
  #undef DOS_BASE_NAME
  #define DOS_BASE_NAME dosbase
  register APTR dosbase __asm("a6") = DOSBase;
#endif

        BPTR lnew=Lock((char *)new,SHARED_LOCK);
        if(lnew) {
          BPTR lold=Lock((char *)old,SHARED_LOCK);
          if(lold)
            ret=SameLock(lold,lnew),UnLock(lold);
          UnLock(lnew);
        }

        if(ret) {
          if(ret==1)
            DeleteFile((char *)new);
          if(ret=0,!Rename((char *)old,(char *)new))
            __seterrno(),ret=-1;
        }

#ifdef IXPATHS
      }
      free(old);
    }
  }
#endif

  return ret;
}
